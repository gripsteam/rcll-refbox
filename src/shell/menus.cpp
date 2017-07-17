
/***************************************************************************
 *  menus.cpp - LLSF RefBox shell menus
 *
 *  Created: Sun Mar 03 00:29:20 2013
 *  Copyright  2013  Tim Niemueller [www.niemueller.de]
 ****************************************************************************/

/*  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the authors nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "menus.h"
#include "colors.h"
#include <boost/format.hpp>
#include <google/protobuf/descriptor.h>

namespace llsfrb_shell {
#if 0 /* just to make Emacs auto-indent happy */
}
#endif

static const int CMD_QUIT   = MAX_COMMAND + 1;
static const int CMD_ACTION = MAX_COMMAND + 2;

Menu::Menu(int nlines, int ncols, int begin_y, int begin_x)
: NCursesMenu(nlines, ncols, begin_y, begin_x),
  stdin_(io_service_, dup(STDIN_FILENO))
{}

NCursesMenuItem*
Menu::operator()(void)
{
  set_current(* (*this)[0]);

  post();
  show();
  refresh();

  nodelay(true);
  start_keyboard();
  io_service_.run();
  io_service_.reset();

  unpost();
  hide();
  refresh();
  if (options() & O_ONEVALUE)
    return current_item();
  else
    return NULL;
}

void
Menu::start_keyboard()
{
  stdin_.async_read_some(boost::asio::null_buffers(),
			 boost::bind(&Menu::handle_keyboard, this,
				     boost::asio::placeholders::error));
}

void
Menu::handle_keyboard(const boost::system::error_code& error)
{
  if (! error) {
    int drvCmnd;
    int err;
    int c;
    bool b_action = FALSE;
    while ((c=getKey()) != ERR) {
      if ((drvCmnd = virtualize(c)) == CMD_QUIT)  break;
      switch((err=driver(drvCmnd))) {
      case E_REQUEST_DENIED:
	On_Request_Denied(c);
	break;
      case E_NOT_SELECTABLE:
	On_Not_Selectable(c);
	break;
      case E_UNKNOWN_COMMAND:
	if (drvCmnd == CMD_ACTION) {
	  if (options() & O_ONEVALUE) {
	    NCursesMenuItem* itm = current_item();
	    assert(itm != 0);
	    if (itm->options() & O_SELECTABLE)
	    {
	      b_action = itm->action();
	      refresh();
	    }
	    else
	      On_Not_Selectable(c);
	  }
	  else {
	    int n = count();
	    for(int i=0; i<n; i++) {
	      NCursesMenuItem* itm = (*this)[i];
	      if (itm->value()) {
		b_action |= itm->action();
		refresh();
	      }
	    }
	  }
	} else
	  On_Unknown_Command(c);
	break;
      case E_NO_MATCH:
	On_No_Match(c);
	break;
      case E_OK:
	break;
      default:
	OnError(err);
      }
    }
    
    if (!b_action) {
      start_keyboard();
    }
  }
}


GenericItemsMenu::GenericItemsMenu(NCursesWindow *parent, int n_items, NCursesMenuItem **items)
  : Menu(n_items + 2, max_cols(n_items, items) + 2,
	 (parent->lines() - max_cols(n_items, items))/2,
	 (parent->cols() - max_cols(n_items, items))/2),
    parent_(parent)
{
  set_mark("");
  InitMenu(items, true, true);
}

void
GenericItemsMenu::On_Menu_Init()
{
  bkgd(' '|COLOR_PAIR(COLOR_DEFAULT));
  //subWindow().bkgd(parent_->getbkgd());
  refresh();
}

int
GenericItemsMenu::max_cols(int n_items, NCursesMenuItem **items)
{
  int rv = 0;
  for (int i = 0; i < n_items; ++i) {
    rv = std::max(rv, (int)strlen(items[i]->name()));
  }
  return rv;
}


MachinePlacingMenu::MachinePlacingMenu(NCursesWindow *parent,
				       std::string machine, std::string puck,
				       bool can_be_placed_under_rfid, bool can_be_loaded_with)
  : Menu(det_lines(can_be_placed_under_rfid, can_be_loaded_with), 30,
	 (parent->lines() - 5)/2, (parent->cols() - 30)/2)
{
  valid_selected_ = false;
  place_under_rfid_ = false;
  s_under_rfid_ = "Place " + puck.substr(0,2) + " under RFID of " + machine;
  s_loaded_with_ = "Load " + machine + " with " + puck.substr(0,2);
  s_cancel_ = "** CANCEL **";
  NCursesMenuItem **mitems = new NCursesMenuItem*[4];

  int idx = 0;

  if (can_be_placed_under_rfid) {
    SignalItem *item = new SignalItem(s_under_rfid_);
    item->signal().connect(boost::bind(&MachinePlacingMenu::item_selected, this, true));
    mitems[idx++] = item;
  }

  if (can_be_loaded_with) {
    SignalItem *item = new SignalItem(s_loaded_with_);
    item->signal().connect(boost::bind(&MachinePlacingMenu::item_selected, this, false));
    mitems[idx++] = item;
  }
  mitems[idx++] = new SignalItem(s_cancel_);
  mitems[idx++] = new NCursesMenuItem();
    

  set_mark("");
  set_format(idx-1, 1);
  InitMenu(mitems, true, true);
  frame("Placing");
}

int
MachinePlacingMenu::det_lines(bool can_be_placed_under_rfid, bool can_be_loaded_with)
{
  int rv = 3;
  if (can_be_placed_under_rfid) rv += 1;
  if (can_be_loaded_with) rv += 1;
  return rv;
}

void
MachinePlacingMenu::item_selected(bool under_rfid)
{
  valid_selected_ = true;
  place_under_rfid_ = under_rfid;
}

bool
MachinePlacingMenu::place_under_rfid()
{
  return place_under_rfid_;
}

void
MachinePlacingMenu::On_Menu_Init()
{
  bkgd(' '|COLOR_PAIR(COLOR_DEFAULT));
  //subWindow().bkgd(parent_->getbkgd());
  refresh();
}

MachinePlacingMenu::operator bool() const
{
  return valid_selected_;
}


TeamSelectMenu::TeamSelectMenu(NCursesWindow *parent,
			       llsf_msgs::Team team,
			       std::shared_ptr<llsf_msgs::GameInfo> gameinfo,
			       std::shared_ptr<llsf_msgs::GameState> gstate)
  : Menu(det_lines(gameinfo, gstate) + 1 + 2, 20 + 2,
	 (parent->lines() - (det_lines(gameinfo, gstate) + 1))/2,
	 (parent->cols() - 20)/2)
{
  team_ = team;

  valid_item_ = false;
  int n_items = det_lines(gameinfo, gstate);
  items_.resize(n_items);
  int ni = 0;
  NCursesMenuItem **mitems = new NCursesMenuItem*[2 + n_items];
  for (int i = 0; i < gameinfo->known_teams_size(); ++i) {
    if (gstate->team_cyan() != gameinfo->known_teams(i) &&
	gstate->team_magenta() != gameinfo->known_teams(i))
    {
      items_[ni++] = gameinfo->known_teams(i);
    }
  }

  std::sort(items_.begin(), items_.end());

  for (int i = 0; i < ni; ++i) {
    SignalItem *item = new SignalItem(items_[i]);
    item->signal().connect(boost::bind(&TeamSelectMenu::team_selected, this, items_[i]));
    mitems[i] = item;
  }
  s_cancel_ = "** CANCEL **";
  mitems[ni] = new SignalItem(s_cancel_);
  mitems[ni+1] = new NCursesMenuItem();

  set_mark("");
  set_format(ni+1, 1);
  InitMenu(mitems, true, true);
}

void
TeamSelectMenu::team_selected(std::string team_name)
{
  valid_item_ = true;
  team_name_ = team_name;
}

std::string
TeamSelectMenu::get_team_name()
{
  return team_name_;
}

void
TeamSelectMenu::On_Menu_Init()
{
  bkgd(' '|COLOR_PAIR(COLOR_DEFAULT));

  if (team_ == llsf_msgs::CYAN) {
    attron(' '|COLOR_PAIR(COLOR_CYAN_ON_BACK));
  } else {
    attron(' '|COLOR_PAIR(COLOR_MAGENTA_ON_BACK));
  }
  box();

  attron(' '|COLOR_PAIR(COLOR_BLACK_ON_BACK)|A_BOLD);
  addstr(0, (width() - 6) / 2, " Team ");
  attroff(A_BOLD);

  refresh();
}

int
TeamSelectMenu::det_lines(std::shared_ptr<llsf_msgs::GameInfo> &gameinfo,
			  std::shared_ptr<llsf_msgs::GameState> &gstate)
{
  int rv = 0;
  for (int i = 0; i < gameinfo->known_teams_size(); ++i) {
    if (gstate->team_cyan() != gameinfo->known_teams(i) &&
	gstate->team_magenta() != gameinfo->known_teams(i))
    {
      ++rv;
    }
  }
  return rv;
}


TeamColorSelectMenu::TeamColorSelectMenu(NCursesWindow *parent)
  : Menu(det_lines() + 1 + 2, 12 + 2,
	 (parent->lines() - (det_lines() + 1))/2,
	 (parent->cols() - 12)/2)
{
  valid_item_ = false;
  int n_items = det_lines();
  items_.resize(n_items);
  int ni = 0;
  NCursesMenuItem **mitems = new NCursesMenuItem*[2 + n_items];

  const google::protobuf::EnumDescriptor *team_enum_desc =
    llsf_msgs::Team_descriptor();
  for (int i = 0; i < n_items; ++i) {
    llsf_msgs::Team team;
    if (llsf_msgs::Team_Parse(team_enum_desc->value(i)->name(), &team)) {
      items_[ni++] = team;
    }
  }

  for (int i = 0; i < ni; ++i) {
    SignalItem *item = new SignalItem(llsf_msgs::Team_Name(items_[i]));
    item->signal().connect(boost::bind(&TeamColorSelectMenu::team_color_selected,
				       this, items_[i]));
    mitems[i] = item;
  }
  s_cancel_ = "** CANCEL **";
  mitems[ni] = new SignalItem(s_cancel_);
  mitems[ni+1] = new NCursesMenuItem();

  set_mark("");
  set_format(ni+1, 1);
  InitMenu(mitems, true, true);
  frame("Team Color");
}

void
TeamColorSelectMenu::team_color_selected(llsf_msgs::Team team_color)
{
  valid_item_ = true;
  team_color_ = team_color;
}

llsf_msgs::Team
TeamColorSelectMenu::get_team_color()
{
  return team_color_;
}

void
TeamColorSelectMenu::On_Menu_Init()
{
  bkgd(' '|COLOR_PAIR(COLOR_DEFAULT));
  //subWindow().bkgd(parent_->getbkgd());
  refresh();
}

int
TeamColorSelectMenu::det_lines()
{
  const google::protobuf::EnumDescriptor *team_enum_desc =
    llsf_msgs::Team_descriptor();
  return team_enum_desc->value_count();
}


RobotMaintenanceMenu::RobotMaintenanceMenu(NCursesWindow *parent, llsf_msgs::Team team,
					   std::shared_ptr<llsf_msgs::RobotInfo> rinfo)
  : Menu(det_lines(team, rinfo) + 1 + 2, det_cols(rinfo) + 2,
	 (parent->lines() - (det_lines(team, rinfo) + 1))/2,
	 (parent->cols() - (det_cols(rinfo) + 2))/2)
{
  team_ = team;
  valid_item_ = false;
  int n_items = det_lines(team, rinfo);
  items_.resize(n_items);
  int ni = 0;
  //bool has_1 = false, has_2 = false, has_3 = false;
  NCursesMenuItem **mitems = new NCursesMenuItem*[2 + n_items];
  for (int i = 0; i < rinfo->robots_size(); ++i) {
    const llsf_msgs::Robot &r = rinfo->robots(i);
    if (r.team_color() != team) continue;

    std::string s = boost::str(boost::format("%s%s %u %s/%s")
			       % (r.state() == llsf_msgs::MAINTENANCE ? "*" : 
				  (r.state() == llsf_msgs::DISQUALIFIED ? "D" : " "))
			       % (r.maintenance_cycles() > 0 ? "!" : " ")
			       % r.number() % r.name() % r.team());
    items_[ni++] = std::make_tuple(s, r.number(),
				   (r.state() != llsf_msgs::MAINTENANCE &&
				    r.state() != llsf_msgs::DISQUALIFIED));

    /*
    if (r.number() == 1)  has_1 = true;
    if (r.number() == 2)  has_2 = true;
    if (r.number() == 3)  has_3 = true;
    */
  }
  /*
  if (! has_1) {
    items_[ni++] = std::make_tuple("  1 0x Unknown", 1, false);
  }
  if (! has_2) {
    items_[ni++] = std::make_tuple("  2 0x Unknown", 2, false);
  }
  if (! has_3) {
    items_[ni++] = std::make_tuple("  3 0x Unknown", 3, false);
  }
  */

  std::sort(items_.begin(), items_.end(),
	    [](const ItemTuple &i1, const ItemTuple &i2) -> bool
	    {
	      return std::get<1>(i1) < std::get<1>(i2);
	    });

  for (int i = 0; i < ni; ++i) {
    SignalItem *item = new SignalItem(std::get<0>(items_[i]));
    item->signal().connect(boost::bind(&RobotMaintenanceMenu::robot_selected, this,
				       std::get<1>(items_[i]), std::get<2>(items_[i])));
    mitems[i] = item;
  }

  int cols = det_cols(rinfo) - std::string("** CANCEL **").length();
  int cols_half = cols / 2;
  s_cancel_ = "";
  for (int i = 0; i < cols_half; ++i) {
    s_cancel_ += " ";
  }
  s_cancel_ += "** CANCEL **";
  for (int i = 0; i < cols_half; ++i) {
    s_cancel_ += " ";
  }
  if (cols_half * 2 < cols) s_cancel_ += " ";
  mitems[ni] = new SignalItem(s_cancel_);
  mitems[ni+1] = new NCursesMenuItem();

  set_mark("");
  set_format(ni+1, 1);
  InitMenu(mitems, true, true);
  frame("Robot Maintenance");
}

void
RobotMaintenanceMenu::robot_selected(unsigned int number, bool maintenance)
{
  valid_item_ = true;
  robot_number_ = number;
  robot_maintenance_ = maintenance;
}

void
RobotMaintenanceMenu::get_robot(unsigned int &number, bool &maintenance)
{
  if (valid_item_) {
    number = robot_number_;
    maintenance = robot_maintenance_;
  }
}

void
RobotMaintenanceMenu::On_Menu_Init()
{
  bkgd(' '|COLOR_PAIR(COLOR_DEFAULT));

  if (team_ == llsf_msgs::CYAN) {
    attron(' '|COLOR_PAIR(COLOR_CYAN_ON_BACK));
  } else {
    attron(' '|COLOR_PAIR(COLOR_MAGENTA_ON_BACK));
  }
  box();

  attron(' '|COLOR_PAIR(COLOR_BLACK_ON_BACK)|A_BOLD);
  addstr(0, (width() - 20) / 2, " Robot Maintenance ");
  attroff(A_BOLD);

  refresh();
}

int
RobotMaintenanceMenu::det_lines(llsf_msgs::Team team,
				std::shared_ptr<llsf_msgs::RobotInfo> &rinfo)
{
  int rv = 0;
  for (int i = 0; i < rinfo->robots_size(); ++i) {
    const llsf_msgs::Robot &r = rinfo->robots(i);
    if (r.team_color() != team) continue;
    rv += 1;
  }
  return rv;
}

int
RobotMaintenanceMenu::det_cols(std::shared_ptr<llsf_msgs::RobotInfo> &rinfo)
{
  int cols = std::string("** CANCEL **").length();
  for (int i = 0; i < rinfo->robots_size(); ++i) {
    const llsf_msgs::Robot &r = rinfo->robots(i);
    cols = std::max(cols, (int)(r.name().length() + r.team().length() + 7));
  }

  return cols;
}



OrderDeliverMenu::OrderDeliverMenu
  (NCursesWindow *parent, llsf_msgs::Team team,
   std::shared_ptr<llsf_msgs::OrderInfo> oinfo,
   std::shared_ptr<llsf_msgs::GameState> gstate)
  : Menu(det_lines(team, oinfo) + 1 + 2, 25 + 2,
	 (parent->lines() - (det_lines(team, oinfo) + 1))/2,
	 (parent->cols() - 26)/2),
    oinfo_(oinfo), team_(team)
{
  order_selected_ = false;
  int n_items = det_lines(team, oinfo);
  items_.resize(n_items);
  int ni = 0;
  NCursesMenuItem **mitems = new NCursesMenuItem*[2 + n_items];
  for (int i = 0; i < oinfo->orders_size(); ++i) {
    const llsf_msgs::Order &o = oinfo->orders(i);

    bool active = (gstate->game_time().sec() >= o.delivery_period_begin() &&
		   gstate->game_time().sec() <= o.delivery_period_end());

    std::string s = boost::str(boost::format("%s %2u: %u x %2s")
			       % (active ? "*" : " ") % o.id() % o.quantity_requested()
			       % llsf_msgs::Order::Complexity_Name(o.complexity()));
    items_[ni++] = std::make_pair(i, s);
  }
  std::sort(items_.begin(), items_.end());

  for (int i = 0; i < ni; ++i) {
    SignalItem *item = new SignalItem(items_[i].second);
    item->signal().connect(boost::bind(&OrderDeliverMenu::order_selected,
				       this, items_[i].first));
    mitems[i] = item;
  }
  s_cancel_ = "** CANCEL **";
  mitems[ni] = new SignalItem(s_cancel_);
  mitems[ni+1] = new NCursesMenuItem();

  set_mark("");
  set_format(ni+1, 1);
  InitMenu(mitems, true, true);
}

void
OrderDeliverMenu::order_selected(int i)
{
  order_selected_ = true;
  order_idx_ = i;
}

const llsf_msgs::Order &
OrderDeliverMenu::order()
{
  return oinfo_->orders(order_idx_);
}

void
OrderDeliverMenu::On_Menu_Init()
{
  bkgd(' '|COLOR_PAIR(COLOR_DEFAULT));

  if (team_ == llsf_msgs::CYAN) {
    attron(' '|COLOR_PAIR(COLOR_CYAN_ON_BACK));
  } else {
    attron(' '|COLOR_PAIR(COLOR_MAGENTA_ON_BACK));
  }
  box();

  attron(' '|COLOR_PAIR(COLOR_BLACK_ON_BACK)|A_BOLD);
  addstr(0, (width() - 8) / 2, " Orders ");
  attroff(A_BOLD);

  for (size_t i = 0; i < items_.size(); ++i) {
    const llsf_msgs::Order &o = oinfo_->orders(items_[i].first);

    if (team_ == llsf_msgs::CYAN) {
      attron(' '|COLOR_PAIR(COLOR_WHITE_ON_CYAN)|A_BOLD);
    } else {
      attron(' '|COLOR_PAIR(COLOR_CYAN_ON_BACK));
    }
    printw(i+1, 14, "%u", o.quantity_delivered_cyan());

    attron(' '|COLOR_PAIR(COLOR_BLACK_ON_BACK));
    addstr(i+1, 15, "/");

    if (team_ == llsf_msgs::MAGENTA) {
      attron(' '|COLOR_PAIR(COLOR_WHITE_ON_MAGENTA)|A_BOLD);
    } else {
      attron(' '|COLOR_PAIR(COLOR_MAGENTA_ON_BACK));
    }
    printw(i+1, 16, "%u", o.quantity_delivered_magenta());


    switch (o.base_color()) {
    case llsf_msgs::BASE_RED:
      attron(' '|COLOR_PAIR(COLOR_WHITE_ON_RED));   break;
    case llsf_msgs::BASE_SILVER:
      attron(' '|COLOR_PAIR(COLOR_BLACK_ON_WHITE)); break;
    case llsf_msgs::BASE_BLACK:
      attron(' '|COLOR_PAIR(COLOR_WHITE_ON_BLACK));   break;
    }
    addstr(i+1, 18, " ");

    for (int j = 0; j < o.ring_colors_size(); ++j) {
      switch (o.ring_colors(j)) {
      case llsf_msgs::RING_BLUE:
	attron(' '|COLOR_PAIR(COLOR_WHITE_ON_BLUE)); break;
      case llsf_msgs::RING_GREEN:
	attron(' '|COLOR_PAIR(COLOR_WHITE_ON_GREEN)); break;
      case llsf_msgs::RING_ORANGE:
	attron(' '|COLOR_PAIR(COLOR_WHITE_ON_ORANGE)); break;
      case llsf_msgs::RING_YELLOW:
	attron(' '|COLOR_PAIR(COLOR_BLACK_ON_YELLOW)); break;
      }
      addstr(i+1, 19+j, " ");
    }

    for (int j = o.ring_colors_size(); j < 4; ++j) {
      attron(' '|COLOR_PAIR(COLOR_BLACK_ON_WHITE));
      addstr(i+1, 19+j, " ");
    }

    switch (o.cap_color()) {
    case llsf_msgs::CAP_BLACK:
      attron(' '|COLOR_PAIR(COLOR_WHITE_ON_BLACK));   break;
    case llsf_msgs::CAP_GREY:
      attron(' '|COLOR_PAIR(COLOR_BLACK_ON_WHITE)); break;
    }
    addstr(i+1, 22, " ");

    attron(' '|COLOR_PAIR(COLOR_BLACK_ON_BACK));
    printw(i+1, 24, "D%u", o.delivery_gate());
  }

  refresh();
}

int
OrderDeliverMenu::det_lines(llsf_msgs::Team team,
			    std::shared_ptr<llsf_msgs::OrderInfo> &oinfo)
{
  if (oinfo) {
    return oinfo->orders_size();
  } else {
    return 0;
  }
}

OrderDeliverMenu::operator bool() const
{
  return order_selected_;
}


OrderByColorDeliverMenu::OrderByColorDeliverMenu
  (NCursesWindow *parent, llsf_msgs::Team team,
   std::shared_ptr<llsf_msgs::OrderInfo> oinfo,
   std::shared_ptr<llsf_msgs::GameState> gstate)
  : Menu(det_lines(team, oinfo) + 2 + 2, 18 + 2,
	 (parent->lines() - (det_lines(team, oinfo) + 2))/2,
	 (parent->cols() - 19)/2),
    oinfo_(oinfo), team_(team)
{
  product_selected_ = false;
  product_idx_ = 0;
  wants_specific_ = false;
  int n_items = det_lines(team, oinfo);
  items_.resize(n_items);
  int ni = 0;
  NCursesMenuItem **mitems = new NCursesMenuItem*[3 + n_items];

  std::list<std::string> orders;

  for (int i = 0; i < oinfo->orders_size(); ++i) {
    const llsf_msgs::Order &o = oinfo->orders(i);

    std::string os = product_spec_to_string(o);
    if (std::find(orders.begin(), orders.end(), os) == orders.end()) {
	    orders.push_back(os);

	    std::string s = boost::str(boost::format("%2s   ")
	                               % llsf_msgs::Order::Complexity_Name(o.complexity()));
	    std::vector<llsf_msgs::RingColor> ring_colors(o.ring_colors_size());
	    for (int i = 0; i < o.ring_colors_size(); ++i) ring_colors[i] = o.ring_colors(i);
	    items_[ni++] = std::make_tuple(o.base_color(), ring_colors, o.cap_color(), s);
    }
  }
  
  std::sort(items_.begin(), items_.end());

  for (int i = 0; i < ni; ++i) {
	  SignalItem *item = new SignalItem(std::get<3>(items_[i]));
	  item->signal().connect(boost::bind(&OrderByColorDeliverMenu::product_selected,
	                                     this, i));
	  mitems[i] = item;
  }

  s_specific_ = "==> Specific";
  SignalItem *item_spec = new SignalItem(s_specific_);
  item_spec->signal().connect(boost::bind(&OrderByColorDeliverMenu::set_wants_specific, this));
  mitems[ni] = item_spec;

  s_cancel_ = "** CANCEL **";
  mitems[ni+1] = new SignalItem(s_cancel_);
  mitems[ni+2] = new NCursesMenuItem();

  set_mark("");
  set_format(ni+2, 1);
  InitMenu(mitems, true, true);
}

void
OrderByColorDeliverMenu::product_selected(int i)
{
  product_selected_ = true;
  product_idx_ = i;
}

void
OrderByColorDeliverMenu::set_wants_specific()
{
	wants_specific_ = true;
}


bool
OrderByColorDeliverMenu::wants_specific()
{
	return wants_specific_;
}

llsf_msgs::BaseColor
OrderByColorDeliverMenu::base_color()
{
	return std::get<0>(items_[product_idx_]);
}

std::vector<llsf_msgs::RingColor>
OrderByColorDeliverMenu::ring_colors()
{
	return std::get<1>(items_[product_idx_]);
}

llsf_msgs::CapColor
OrderByColorDeliverMenu::cap_color()
{
	return std::get<2>(items_[product_idx_]);
}

void
OrderByColorDeliverMenu::On_Menu_Init()
{
  bkgd(' '|COLOR_PAIR(COLOR_DEFAULT));

  if (team_ == llsf_msgs::CYAN) {
    attron(' '|COLOR_PAIR(COLOR_CYAN_ON_BACK));
  } else {
    attron(' '|COLOR_PAIR(COLOR_MAGENTA_ON_BACK));
  }
  box();

  attron(' '|COLOR_PAIR(COLOR_BLACK_ON_BACK)|A_BOLD);
  addstr(0, (width() - 8) / 2, " Orders ");
  attroff(A_BOLD);

  for (size_t i = 0; i < items_.size(); ++i) {
	  switch (std::get<0>(items_[i])) {
    case llsf_msgs::BASE_RED:
      attron(' '|COLOR_PAIR(COLOR_WHITE_ON_RED));   break;
    case llsf_msgs::BASE_SILVER:
      attron(' '|COLOR_PAIR(COLOR_BLACK_ON_WHITE)); break;
    case llsf_msgs::BASE_BLACK:
      attron(' '|COLOR_PAIR(COLOR_WHITE_ON_BLACK));   break;
    }
    addstr(i+1, 14, " ");

    for (size_t j = 0; j < std::get<1>(items_[i]).size(); ++j) {
	    switch (std::get<1>(items_[i]).at(j)) {
      case llsf_msgs::RING_BLUE:
	attron(' '|COLOR_PAIR(COLOR_WHITE_ON_BLUE)); break;
      case llsf_msgs::RING_GREEN:
	attron(' '|COLOR_PAIR(COLOR_WHITE_ON_GREEN)); break;
      case llsf_msgs::RING_ORANGE:
	attron(' '|COLOR_PAIR(COLOR_WHITE_ON_ORANGE)); break;
      case llsf_msgs::RING_YELLOW:
	attron(' '|COLOR_PAIR(COLOR_BLACK_ON_YELLOW)); break;
      }
      addstr(i+1, 15+j, " ");
    }

    for (int j = std::get<1>(items_[i]).size(); j < 4; ++j) {
      attron(' '|COLOR_PAIR(COLOR_BLACK_ON_WHITE));
      addstr(i+1, 15+j, " ");
    }

    switch (std::get<2>(items_[i])) {
    case llsf_msgs::CAP_BLACK:
      attron(' '|COLOR_PAIR(COLOR_WHITE_ON_BLACK));   break;
    case llsf_msgs::CAP_GREY:
      attron(' '|COLOR_PAIR(COLOR_BLACK_ON_WHITE)); break;
    }
    addstr(i+1, 18, " ");

    attron(' '|COLOR_PAIR(COLOR_BLACK_ON_BACK));
  }

  refresh();
}


std::string
OrderByColorDeliverMenu::product_spec_to_string(const llsf_msgs::Order &o)
{
	std::string rv = llsf_msgs::BaseColor_Name(o.base_color()) + "-";

  for (int i = 0; i < o.ring_colors_size(); ++i) {
	  if (i > 0)  rv += "|";
	  rv += llsf_msgs::RingColor_Name(o.ring_colors(i));
  }

  rv += "-" + llsf_msgs::CapColor_Name(o.cap_color());
  return rv;
}

int
OrderByColorDeliverMenu::det_lines(llsf_msgs::Team team,
                                   std::shared_ptr<llsf_msgs::OrderInfo> &oinfo)
{
  if (oinfo) {
	  int num_products = 0;
	  std::list<std::string> orders;
	  for (int i = 0; i < oinfo->orders_size(); ++i) {
		  std::string os = product_spec_to_string(oinfo->orders(i));
		  if (std::find(orders.begin(), orders.end(), os) == orders.end()) {
			  orders.push_back(os);
			  num_products += 1;
		  }
	  }
	  return num_products;
  } else {
    return 0;
  }
}

OrderByColorDeliverMenu::operator bool() const
{
  return product_selected_;
}


} // end of namespace llsfrb

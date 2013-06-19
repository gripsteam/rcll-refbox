/***************************************************************************
 *  MainWindow.cpp - Central GUI element for the LLSFVIS
 *
 *  Created: Mon Jan 31 17:46:35 2013
 *  Copyright  2013  Daniel Ewert (daniel.ewert@ima.rwth-aachen.de)
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

#include "MainWindow.h"
#include <iostream>
#include <list>
#include "PlacePuckDialog.h"

namespace LLSFVis {

MainWindow::MainWindow() :
		//_playFieldTabGrid(Gtk::ORIENTATION_HORIZONTAL),
		loggingTabPaned_(Gtk::ORIENTATION_HORIZONTAL), buttonBoxPlayField_(
				Gtk::ORIENTATION_VERTICAL), buttonBoxLogging_(
				Gtk::ORIENTATION_VERTICAL) {

	have_machineinfo_ = false;
	have_puckinfo_ = false;

	//SETUP the GUI

	set_default_size(750, 750);
	set_position(Gtk::WIN_POS_CENTER);
	set_title("Robocup Logistics League");
	tabs_.set_border_width(10);

	//the PlayField tab
	addPuckToMachineButton_.set_label("Add Puck to Machine...");
	addPuckToMachineButton_.signal_clicked().connect(
			sigc::mem_fun(*this,
					&MainWindow::on_add_puck_to_machine_button_clicked));
	setPuckUnderRFIDButton_.set_label("Place Puck under RFID...");
	setPuckUnderRFIDButton_.signal_clicked().connect(
			sigc::mem_fun(*this,
					&MainWindow::on_set_puck_under_rfid_button_clicked));
	startPauseButton_.set_label("Start Game");
	startPauseButton_.signal_clicked().connect(
			sigc::mem_fun(*this, &MainWindow::on_start_pause_button_clicked));
	playFieldButton4_.set_label("Button4");

	buttonBoxPlayField_.pack_start(addPuckToMachineButton_, Gtk::PACK_SHRINK);
	buttonBoxPlayField_.pack_start(setPuckUnderRFIDButton_, Gtk::PACK_SHRINK);
	buttonBoxPlayField_.pack_start(startPauseButton_, Gtk::PACK_SHRINK);
	buttonBoxPlayField_.pack_start(playFieldButton4_, Gtk::PACK_SHRINK);

	logPreviewScrollWindow_.add(logPreviewWidget_);

	Pango::FontDescription font;
	font.set_size(Pango::SCALE * 18);
	font.set_weight(Pango::WEIGHT_BOLD);
	attentionMsg_.override_color(Gdk::RGBA("dark red"));
	attentionMsg_.override_font(font);

	playFieldTabGrid_.set_row_spacing(5);
	playFieldTabGrid_.set_column_spacing(5);
	playFieldTabGrid_.attach(attentionMsg_, 0, 0, 2, 1);
	playFieldTabGrid_.attach_next_to(aspectFrame_, attentionMsg_,
			Gtk::POS_BOTTOM, 1, 2);
	playFieldTabGrid_.attach_next_to(orderWidget_, aspectFrame_, Gtk::POS_RIGHT,
			1, 2);
	playFieldTabGrid_.attach_next_to(pucksWidget_, orderWidget_, Gtk::POS_RIGHT,
			1, 2);
	//playFieldTabGrid_.attach_next_to(buttonBoxPlayField_, pucksWidget_,
	//		Gtk::POS_RIGHT, 1, 1);
	playFieldTabGrid_.attach_next_to(logPreviewScrollWindow_, aspectFrame_,
			Gtk::POS_BOTTOM, 2, 1);
	//playFieldTabGrid_.attach_next_to(stateWidget_, buttonBoxPlayField_,
	//		Gtk::POS_BOTTOM, 1, 1);

	//Where normally the buttons would be
	playFieldTabGrid_.attach_next_to(stateWidget_, pucksWidget_,
				Gtk::POS_RIGHT, 1, 1);
	playFieldTabGrid_.attach_next_to(rulesWidget_,aspectFrame_,Gtk::POS_LEFT,1,3);

	//Create the logging tab
	logButton1_.set_label("log1");
	logButton2_.set_label("log2");
	logButton3_.set_label("log3");
	logButton4_.set_label("log4");

	buttonBoxLogging_.pack_start(logButton1_, Gtk::PACK_SHRINK);
	buttonBoxLogging_.pack_start(logButton2_, Gtk::PACK_SHRINK);
	buttonBoxLogging_.pack_start(logButton3_, Gtk::PACK_SHRINK);
	buttonBoxLogging_.pack_start(logButton4_, Gtk::PACK_SHRINK);

	//make both logviews use the same model
	logPreviewWidget_.set_model(logWidget_.get_model());
	logWidget_.set_hexpand(true);
	logPreviewWidget_.set_hexpand(true);
	logScrollWindow_.add(logWidget_);
	loggingTabPaned_.pack1(logScrollWindow_, true, true);
	loggingTabPaned_.pack2(buttonBoxLogging_, false, false);

	tabs_.append_page(playFieldTabGrid_, "Playfield");
	tabs_.append_page(loggingTabPaned_, "RefBox Log");
	add(tabs_);
	aspectFrame_.set(Gtk::ALIGN_START, Gtk::ALIGN_START, 1, true);
	aspectFrame_.add(playFieldWidget_);
	aspectFrame_.set_border_width(0);
	aspectFrame_.set_shadow_type(Gtk::SHADOW_NONE);
	playFieldWidget_.set_size_request(600, 600);
	playFieldWidget_.set_hexpand(true);
	playFieldWidget_.set_vexpand(true);
	logScrollWindow_.set_size_request(600, 600);
	logPreviewScrollWindow_.set_size_request(0, 150);
	buttonBoxPlayField_.set_size_request(150, 0);
	buttonBoxLogging_.set_size_request(150, 0);
	show_all_children();
}

void MainWindow::add_log_message(std::string msg) {
	logWidget_.add_log_message(msg);
}

MainWindow::~MainWindow() {

}

void MainWindow::update_game_state(llsf_msgs::GameState& gameState) {
	stateWidget_.update_game_state(gameState);
	orderWidget_.update_game_time(gameState.game_time());
}

void MainWindow::set_attention_msg(llsf_msgs::AttentionMessage& msg) {
	attentionMsg_.set_text(msg.message());
	int timeToShow = 30000;
	if (msg.has_time_to_show()) {
		timeToShow = msg.time_to_show() * 1000;
	}
	Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &MainWindow::clear_attention_msg), timeToShow);
}

void MainWindow::update_machines(llsf_msgs::MachineInfo& mSpecs) {
	machines_.CopyFrom(mSpecs);
	have_machineinfo_=true;
	playFieldWidget_.update_machines(mSpecs);
}

void MainWindow::update_pucks(const llsf_msgs::PuckInfo& pucks) {
	pucks_.CopyFrom(pucks);
	have_puckinfo_=true;
	playFieldWidget_.update_pucks(pucks);
	pucksWidget_.update_pucks(pucks);
}

bool MainWindow::clear_attention_msg() {
	attentionMsg_.set_text("");
	return true;
}

void MainWindow::update_robots(llsf_msgs::RobotInfo& robotInfo) {
	stateWidget_.update_robot_info(robotInfo);
	playFieldWidget_.update_robot_info(robotInfo);
}

void MainWindow::on_add_puck_to_machine_button_clicked() {
	find_free_pucks();

	PlacePuckDialog pd(freePucks_, machines_);
	int resp = pd.run();
	if (resp == Gtk::RESPONSE_OK) {
		const llsf_msgs::Machine& m = pd.get_selected_machine();
		const llsf_msgs::Puck& p = pd.get_selected_puck();
		llsf_msgs::PlacePuckUnderMachine ppum;
		ppum.set_machine_name(m.name());
		ppum.set_puck_id(p.id());
		std::cout << ppum.DebugString() << std::endl;
		signal_place_puck_under_machine_.emit(ppum);
	}

}

void MainWindow::on_set_puck_under_rfid_button_clicked() {
	//TODO tbd
}

void MainWindow::on_start_pause_button_clicked() {
	llsf_msgs::SetGameState gsm;
	if (startPauseButton_.get_label() == "Start Game") {
		startPauseButton_.set_label("Pause Game");
		gsm.set_state(llsf_msgs::GameState::RUNNING);

	} else if (startPauseButton_.get_label() == "Pause Game") {
		startPauseButton_.set_label("Continue Game");
		gsm.set_state(llsf_msgs::GameState::PAUSED);
	} else if (startPauseButton_.get_label() == "Continue Game") {
		startPauseButton_.set_label("Pause Game");
		gsm.set_state(llsf_msgs::GameState::RUNNING);

	}
	signal_set_game_state_.emit(gsm);
}

void MainWindow::update_orders(const llsf_msgs::OrderInfo& orderInfo) {
	orderWidget_.update_orders(orderInfo);
}

llsf_msgs::PuckInfo MainWindow::find_free_pucks() {
	freePucks_.Clear();

	if (have_machineinfo_ && have_puckinfo_) {
		for (int puck_index = 0; puck_index < pucks_.pucks_size();
				++puck_index) {
			bool found = false;
			for (int machine_index = 0;
					machine_index < machines_.machines_size();
					++machine_index) {
				for (int loaded_puck_index = 0;
						loaded_puck_index
								< machines_.machines(machine_index).loaded_with_size();
						++loaded_puck_index) {
					if (pucks_.pucks(puck_index).id()
							== machines_.machines(machine_index).loaded_with(
									loaded_puck_index).id()) {
						found = true;
						break;
					}
				}
				if (found)
					break;
			}

			if (!found) {
				llsf_msgs::Puck* puckCopy = freePucks_.add_pucks();
				puckCopy->CopyFrom(pucks_.pucks(puck_index));
			}
		}

	}
	return freePucks_;
}

} /* namespace LLSFVis */

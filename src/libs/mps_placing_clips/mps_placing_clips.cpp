
/***************************************************************************
 *  mps_placing_clips.cpp - mps_placing generator for CLIPS
 *
 *  Created: Tue Apr 16 13:51:14 2013
 *  Copyright  2013  Tim Niemueller [www.niemueller.de]
 *             2017  Tobias Neumann
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

#include <mps_placing_clips/mps_placing_clips.h>

#include <core/threading/mutex_locker.h>

namespace mps_placing_clips {
#if 0 /* just to make Emacs auto-indent happy */
}
#endif

/** @class MPSPlacingGenerator <protobuf_clips/communicator.h>
 * CLIPS protobuf integration class.
 * This class adds functionality related to protobuf to a given CLIPS
 * environment. It supports the creation of communication channels
 * through protobuf_comm. An instance maintains its own message register
 * shared among server, peer, and clients.
 * @author Tim Niemueller
 */

/** Constructor.
 * @param env CLIPS environment to which to provide the protobuf functionality
 * @param env_mutex mutex to lock when operating on the CLIPS environment.
 */
MPSPlacingGenerator::MPSPlacingGenerator(CLIPS::Environment *env,
						     fawkes::Mutex &env_mutex)
  : clips_(env), clips_mutex_(env_mutex)
{
  setup_clips();
  generation_is_running = false;
}

/** Destructor. */
MPSPlacingGenerator::~MPSPlacingGenerator()
{
  {
    fawkes::MutexLocker lock(&clips_mutex_);

    for (auto f : functions_) {
      clips_->remove_function(f);
    }
    functions_.clear();
  }
}


#define ADD_FUNCTION(n, s)						\
  clips_->add_function(n, s);						\
  functions_.push_back(n);


/** Setup CLIPS environment. */
void
MPSPlacingGenerator::setup_clips()
{
  fawkes::MutexLocker lock(&clips_mutex_);

  ADD_FUNCTION("mps-generator-start", (sigc::slot<void>(sigc::mem_fun(*this, &MPSPlacingGenerator::generate_start))));
  ADD_FUNCTION("mps-generator-abort", (sigc::slot<void>(sigc::mem_fun(*this, &MPSPlacingGenerator::generate_abort))));
  ADD_FUNCTION("mps-generator-running", (sigc::slot<CLIPS::Value>(sigc::mem_fun(*this, &MPSPlacingGenerator::generate_running))));
  ADD_FUNCTION("mps-generator-field-generated", (sigc::slot<CLIPS::Value>(sigc::mem_fun(*this, &MPSPlacingGenerator::field_layout_generated))));
  ADD_FUNCTION("mps-generator-get-generated-field", (sigc::slot<CLIPS::Values>(sigc::mem_fun(*this, &MPSPlacingGenerator::get_generated_field))));
}

void
MPSPlacingGenerator::generate_start()
{

}

void
MPSPlacingGenerator::generate_abort()
{

}

bool
MPSPlacingGenerator::generate_running()
{
  return false;
}

bool
MPSPlacingGenerator::field_layout_generated()
{
  return false;
}

CLIPS::Values
MPSPlacingGenerator::get_generated_field()
{
  return CLIPS::Values(1, CLIPS::Value("INVALID-GENERATION", CLIPS::TYPE_SYMBOL));
}

} // end namespace protobuf_clips
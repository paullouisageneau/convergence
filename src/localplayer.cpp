/***************************************************************************
 *   Copyright (C) 2017-2018 by Paul-Louis Ageneau                         *
 *   paul-louis (at) ageneau (dot) org                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include "src/localplayer.hpp"

#include "pla/binaryformatter.hpp"

using pla::BinaryFormatter;

namespace convergence {

LocalPlayer::LocalPlayer(sptr<MessageBus> messageBus) : Player(messageBus, messageBus->localId()) {
	mOldYaw = mYaw;
	mOldPitch = mPitch;
	mOldSpeed = 0.f;
	mOldIsJumping = false;

	mPositionUpdateCooldown = 0.f;
}

LocalPlayer::~LocalPlayer(void) {}

void LocalPlayer::update(sptr<Collidable> terrain, double time) {
	Player::update(terrain, time);

	mPositionUpdateCooldown += time;

	if (mPositionUpdateCooldown > 0.5 || std::abs(mYaw - mOldYaw) > Epsilon ||
	    std::abs(mPitch - mOldPitch) > Epsilon || std::abs(mSpeed - mOldSpeed) > Epsilon ||
	    mIsJumping != mOldIsJumping) {
		mPositionUpdateCooldown = 0.;
		mOldYaw = mYaw;
		mOldPitch = mPitch;
		mOldSpeed = mSpeed;
		mOldIsJumping = mIsJumping;

		sendPosition();
		sendControl();
	}
}

void LocalPlayer::sendPosition(void) {
	Message message(Message::PlayerPosition);
	BinaryFormatter formatter;
	formatter << float32_t(mPosition.x);
	formatter << float32_t(mPosition.y);
	formatter << float32_t(mPosition.z);
	message.payload = formatter.data();
	mMessageBus->send(message);
}

void LocalPlayer::sendControl(void) {
	Message message(Message::PlayerControl);
	BinaryFormatter formatter;
	formatter << float32_t(mYaw) << float32_t(mPitch);
	formatter << float32_t(mSpeed);
	formatter << float32_t(mGravity);
	formatter << uint32_t(mIsJumping ? 0x1 : 0x0);
	message.payload = formatter.data();
	mMessageBus->send(message);
}

void LocalPlayer::onMessage(const Message &message) {
	switch (message.type) {
	case Message::PlayerPosition:
		// Ignore
		break;

	case Message::PlayerControl:
		// Ignore
		break;

	default:
		Player::onMessage(message);
		break;
	}
}

} // namespace convergence

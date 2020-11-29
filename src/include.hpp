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

#ifndef CONVERGENCE_INCLUDE_H
#define CONVERGENCE_INCLUDE_H

#include "pla/binary.hpp"
#include "pla/include.hpp"
#include "pla/linalg.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <tuple>

namespace convergence {

using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
template <typename T> using sptr = shared_ptr<T>;
template <typename T> using wptr = weak_ptr<T>;
template <typename T> using uptr = unique_ptr<T>;

using std::nullopt;
using std::optional;
using std::pair;
using std::tuple;

using std::mutex;
using std::recursive_mutex;

using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::quat;

using glm::dmat2;
using glm::dmat3;
using glm::dmat4;
using glm::dvec2;
using glm::dvec3;
using glm::dvec4;

using glm::dquat;

using pla::byte;
using pla::float32_t;
using pla::float64_t;
using pla::int16_t;
using pla::int32_t;
using pla::int64_t;
using pla::int8_t;
using pla::uint16_t;
using pla::uint32_t;
using pla::uint64_t;
using pla::uint8_t;

using pla::binary;
using pla::binary_hash;
using pla::string;

using pla::hash_combine;
using pla::to_binary;
using pla::to_hex;
using pla::to_integer;

using pla::Epsilon;
using pla::Pi;
using pla::Sqrt2;

} // namespace convergence

#endif

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2016,  The University of Memphis,
 *                           Regents of the University of California,
 *                           Arizona Board of Regents.
 *
 * This file is part of NLSR (Named-data Link State Routing).
 * See AUTHORS.md for complete list of NLSR authors and contributors.
 *
 * NLSR is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * NLSR is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * NLSR, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "coordinate-lsa.hpp"
#include "tlv-nlsr.hpp"

#include <ndn-cxx/util/concepts.hpp>
#include <ndn-cxx/encoding/block-helpers.hpp>

namespace nlsr {
namespace tlv  {

BOOST_CONCEPT_ASSERT((ndn::WireEncodable<CoordinateLsa>));
BOOST_CONCEPT_ASSERT((ndn::WireDecodable<CoordinateLsa>));
static_assert(std::is_base_of<ndn::tlv::Error, CoordinateLsa::Error>::value,
              "CoordinateLsa::Error must inherit from tlv::Error");

CoordinateLsa::CoordinateLsa()
  : m_hyperbolicRadius(0.0)
  , m_hyperbolicAngle(0.0)
{
}

CoordinateLsa::CoordinateLsa(const ndn::Block& block)
{
  wireDecode(block);
}

template<ndn::encoding::Tag TAG>
size_t
CoordinateLsa::wireEncode(ndn::EncodingImpl<TAG>& block) const
{
  size_t totalLength = 0;
  size_t doubleLength = 10;

  std::vector<double>::const_iterator it = m_hyperbolicAngle.begin();
  for (; it != m_hyperbolicAngle.end(); ++it) {
    const uint8_t* doubleBytes1 = reinterpret_cast<const uint8_t*>(&m_hyperbolicAngle[*it]);
    totalLength += block.prependByteArrayBlock(ndn::tlv::nlsr::Double, doubleBytes1, 8);
    totalLength += block.prependVarNumber(doubleLength);
    totalLength += block.prependVarNumber(ndn::tlv::nlsr::HyperbolicAngle);
  }

  const uint8_t* doubleBytes2 = reinterpret_cast<const uint8_t*>(&m_hyperbolicRadius);
  totalLength += block.prependByteArrayBlock(ndn::tlv::nlsr::Double, doubleBytes2, 8);
  totalLength += block.prependVarNumber(doubleLength);
  totalLength += block.prependVarNumber(ndn::tlv::nlsr::HyperbolicRadius);

  totalLength += m_lsaInfo.wireEncode(block);

  totalLength += block.prependVarNumber(totalLength);
  totalLength += block.prependVarNumber(ndn::tlv::nlsr::CoordinateLsa);

  return totalLength;
}

template size_t
CoordinateLsa::wireEncode<ndn::encoding::EncoderTag>(ndn::EncodingImpl<ndn::encoding::EncoderTag>& block) const;

template size_t
CoordinateLsa::wireEncode<ndn::encoding::EstimatorTag>(ndn::EncodingImpl<ndn::encoding::EstimatorTag>& block) const;

const ndn::Block&
CoordinateLsa::wireEncode() const
{
  if (m_wire.hasWire()) {
    return m_wire;
  }

  ndn::EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  ndn::EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();

  return m_wire;
}

void
CoordinateLsa::wireDecode(const ndn::Block& wire)
{
  m_hyperbolicRadius = 0.0;
  //m_hyperbolicAngle(0.0);

  m_wire = wire;

  if (m_wire.type() != ndn::tlv::nlsr::CoordinateLsa) {
    std::stringstream error;
    error << "Expected CoordinateLsa Block, but Block is of a different type: #"
          << m_wire.type();
    throw Error(error.str());
  }

  m_wire.parse();

  ndn::Block::element_const_iterator val = m_wire.elements_begin();

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::LsaInfo) {
    m_lsaInfo.wireDecode(*val);
    ++val;
  }
  else {
    throw Error("Missing required LsaInfo field");
  }

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::HyperbolicRadius) {
    val->parse();
    ndn::Block::element_const_iterator it = val->elements_begin();
    if (it != val->elements_end() && it->type() == ndn::tlv::nlsr::Double) {
      m_hyperbolicRadius = *reinterpret_cast<const double*>(it->value());
    }
    else {
      throw Error("HyperbolicRadius: Missing required Double field");
    }
  }
  else {
    throw Error("Missing required HyperbolicRadius field");
  }

  if (val != m_wire.elements_end() && val->type() == ndn::tlv::nlsr::HyperbolicAngle) {
    val->parse();
    ndn::Block::element_const_iterator it = val->elements_begin();
    // if (it != val->elements_end() && it->type() == ndn::tlv::nlsr::Double) {
    //   m_hyperbolicAngle = *reinterpret_cast<const double*>(it->value());
    // }
    // else {
    //   throw Error("HyperbolicAngle: Missing required Double field");
    // }
    for(int i = 0; it != val->elements_end(); ++it, ++i) {
      if (it != val->elements_end() && it->type() == ndn::tlv::nlsr::Double) {
        m_hyperbolicAngle[i] = *reinterpret_cast<const double*>(it->value());
      }
      else {
        throw Error("HyperbolicAngle: Missing required Double field");
      }
    }

    ++val;
  }
  else {
    throw Error("Missing required HyperbolicAngle field");
  }
}

std::ostream&
operator<<(std::ostream& os, const CoordinateLsa& coordinateLsa)
{
  os << "CoordinateLsa("
     << coordinateLsa.getLsaInfo() << ", "
     << "HyperbolicRadius: " << coordinateLsa.getHyperbolicRadius() << ", "
     /*<< "HyperbolicAngle: " << coordinateLsa.getHyperbolicAngle() << ")"*/;

  return os;
}

} // namespace tlv
} // namespace nlsr

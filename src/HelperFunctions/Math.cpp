/* Copyright 2013-2019 Homegear GmbH
 *
 * libhomegear-base is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * 
 * libhomegear-base is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with libhomegear-base.  If not, see
 * <http://www.gnu.org/licenses/>.
 * 
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU Lesser General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
*/

#include "Math.h"
#include "HelperFunctions.h"

#include <iomanip>

namespace BaseLib {

Math::Point2D::Point2D(const std::string &s) {
  std::vector<std::string> elements = HelperFunctions::splitAll(s, ';');
  if (elements.size() >= 2) {
    x = Math::getDouble(elements[0]);
    y = Math::getDouble(elements[1]);
  }
}

Math::Point3D::Point3D(const std::string &s) {
  std::vector<std::string> elements = HelperFunctions::splitAll(s, ';');
  if (elements.size() >= 3) {
    x = Math::getDouble(elements[0]);
    y = Math::getDouble(elements[1]);
    z = Math::getDouble(elements[2]);
  }
}

void Math::Line::closestPointToPoint(const Point2D &p, Point2D &r) {
  Vector2D AB(_b.x - _a.x, _b.y - _a.y);
  Vector2D AP(p.x - _a.x, p.y - _a.y);

  float abTimes2 = AB.x * AB.x + AB.y * AB.y;
  float apTimesAb = AP.x * AB.x + AP.y * AB.y;

  float t = apTimesAb / abTimes2;

  if (t < 0) t = 0;
  else if (t > 1) t = 1;

  r.x = _a.x + AB.x * t;
  r.y = _a.y + AB.y * t;
}

void Math::Matrix3x3::inverse(Matrix3x3 &inversedMatrix) {
  double det = determinant();
  if (det == 0) return;
  double s = 1.0 / det;

  inversedMatrix.p00 = s * (p11 * p22 - p12 * p21);
  inversedMatrix.p10 = s * (p12 * p20 - p10 * p22);
  inversedMatrix.p20 = s * (p10 * p21 - p11 * p20);

  inversedMatrix.p01 = s * (p02 * p21 - p01 * p22);
  inversedMatrix.p11 = s * (p00 * p22 - p02 * p20);
  inversedMatrix.p21 = s * (p01 * p20 - p00 * p21);

  inversedMatrix.p02 = s * (p01 * p12 - p02 * p11);
  inversedMatrix.p12 = s * (p02 * p10 - p00 * p12);
  inversedMatrix.p22 = s * (p00 * p11 - p01 * p10);
}

Math::Vector3D Math::Matrix3x3::operator*(const Math::Vector3D &rhs) const {
  Math::Vector3D result;
  result.x = p00 * rhs.x + p01 * rhs.y + p02 * rhs.z;
  result.y = p10 * rhs.x + p11 * rhs.y + p12 * rhs.z;
  result.z = p20 * rhs.x + p21 * rhs.y + p22 * rhs.z;
  return result;
}

double Math::Matrix3x3::determinant() {
  return (p00 * (p11 * p22 - p12 * p21)) - (p01 * (p10 * p22 - p12 * p20)) + (p02 * (p10 * p21 - p11 * p20));
}

std::string Math::Matrix3x3::toString() {
  std::string result = std::to_string(p00) + '\t' + std::to_string(p01) + '\t' + std::to_string(p02) + "\r\n";
  result += std::to_string(p10) + '\t' + std::to_string(p11) + '\t' + std::to_string(p12) + "\r\n";
  result += std::to_string(p20) + '\t' + std::to_string(p21) + '\t' + std::to_string(p22) + "\r\n";
  return result;
}

double Math::Triangle::distance(const Point2D &p, Point2D *closestPoint) {
  //Source is: http://wonderfl.net/c/b27F

  Vector2D AB(_b.x - _a.x, _b.y - _a.y);
  Vector2D AP(p.x - _a.x, p.y - _a.y);
  double ABXAP = (AB.x * AP.y) - (AB.y * AP.x);

  Vector2D BC(_c.x - _b.x, _c.y - _b.y);
  Vector2D BP(p.x - _b.x, p.y - _b.y);
  double BCXBP = (BC.x * BP.y) - (BC.y * BP.x);

  Vector2D CA(_a.x - _c.x, _a.y - _c.y);
  Vector2D CP(p.x - _c.x, p.y - _c.y);
  double CAXCP = (CA.x * CP.y) - (CA.y * CP.x);

  double distance = 0;

  //+++ //inside
  if (ABXAP >= 0 && BCXBP >= 0 && CAXCP >= 0) {
    if (closestPoint) {
      closestPoint->x = p.x;
      closestPoint->y = p.y;
    }
  }
    //-+- //vertex
  else if (ABXAP < 0 && BCXBP >= 0 && CAXCP < 0) {
    distance = AP.x * AP.x + AP.y * AP.y;
    if (closestPoint) {
      closestPoint->x = _a.x;
      closestPoint->y = _a.y;
    }
  }
    //--+ //vertex
  else if (ABXAP < 0 && BCXBP < 0 && CAXCP >= 0) {
    distance = BP.x * BP.x + BP.y * BP.y;
    if (closestPoint) {
      closestPoint->x = _b.x;
      closestPoint->y = _b.y;
    }
  }
    //+-- //vertex
  else if (ABXAP >= 0 && BCXBP < 0 && CAXCP < 0) {
    distance = CP.x * CP.x + CP.y * CP.y;
    if (closestPoint) {
      closestPoint->x = _c.x;
      closestPoint->y = _c.y;
    }
  }
    //-++ //edge
  else if (ABXAP < 0 && BCXBP >= 0 && CAXCP >= 0) {
    double wd = ((AB.x * AP.x) + (AB.y * AP.y)) / ((AB.x * AB.x) + (AB.y * AB.y));
    if (wd < 0) { wd = 0; }
    if (wd > 1) { wd = 1; }
    Point2D r;
    r.x = _a.x + (_b.x - _a.x) * wd;
    r.y = _a.y + (_b.y - _a.y) * wd;

    if (closestPoint) {
      closestPoint->x = r.x;
      closestPoint->y = r.y;
    }
    r.x = p.x - r.x;
    r.y = p.y - r.y;
    distance = r.x * r.x + r.y * r.y;
  }
    //+-+ //edge
  else if (ABXAP >= 0 && BCXBP < 0 && CAXCP >= 0) {
    double wd = ((BC.x * BP.x) + (BC.y * BP.y)) / ((BC.x * BC.x) + (BC.y * BC.y));
    if (wd < 0) { wd = 0; }
    if (wd > 1) { wd = 1; }
    Point2D r;
    r.x = _b.x + (_c.x - _b.x) * wd;
    r.y = _b.y + (_c.y - _b.y) * wd;

    if (closestPoint) {
      closestPoint->x = r.x;
      closestPoint->y = r.y;
    }
    r.x = p.x - r.x;
    r.y = p.y - r.y;
    distance = r.x * r.x + r.y * r.y;
  }
    //++- //edge
  else if (ABXAP >= 0 && BCXBP >= 0 && CAXCP < 0) {
    double wd = ((CA.x * CP.x) + (CA.y * CP.y)) / ((CA.x * CA.x) + (CA.y * CA.y));
    if (wd < 0) { wd = 0; }
    if (wd > 1) { wd = 1; }
    Point2D r;
    r.x = _c.x + (_a.x - _c.x) * wd;
    r.y = _c.y + (_a.y - _c.y) * wd;

    if (closestPoint) {
      closestPoint->x = r.x;
      closestPoint->y = r.y;
    }
    r.x = p.x - r.x;
    r.y = p.y - r.y;
    distance = r.x * r.x + r.y * r.y;
  }

  return distance;
}

const std::array<int32_t, 23> Math::_asciiToBinaryTable{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15};

bool Math::isNumber(const std::string &s, bool hex) {
  if (!hex) hex = (s.find('x') != std::string::npos);
  if (!hex) {
    auto c = s.c_str();
    if (*c == '-') c++;
    do {
      if (std::isdigit(*c) == 0) return false;
      c++;
    } while (*c != '\0');
  } else try { std::stoll(s, 0, 16); } catch (...) { return false; }
  return true;
}

int32_t Math::getNumber(const std::string &s, bool isHex) {
  auto xpos = s.find('x');
  int32_t number = 0;
  //Don't change to std::stol, because otherwise numbers larger than 0x7FFFFFFF can't be parsed.
  if (xpos == std::string::npos && !isHex) try { number = (int32_t)std::stoll(s, 0, 10); } catch (...) {}
  else try { number = (int32_t)std::stoll(s, 0, 16); } catch (...) {}
  return number;
}

int64_t Math::getNumber64(const std::string &s, bool isHex) {
  auto xpos = s.find('x');
  int64_t number = 0;
  if (xpos == std::string::npos && !isHex) try { number = std::stoll(s, 0, 10); } catch (...) {}
  else try { number = std::stoll(s, 0, 16); } catch (...) {}
  return number;
}

int32_t Math::getNumber(char hexChar) {
  return _asciiToBinaryTable[std::toupper(hexChar) - '0'];
}

uint32_t Math::getUnsignedNumber(const std::string &s, bool isHex) {
  auto xpos = s.find('x');
  uint32_t number = 0;
  if (xpos == std::string::npos && !isHex) try { number = (uint32_t)std::stoul(s, 0, 10); } catch (...) {}
  else try { number = (uint32_t)std::stoul(s, 0, 16); } catch (...) {}
  return number;
}

uint64_t Math::getUnsignedNumber64(const std::string &s, bool isHex) {
  auto xpos = s.find('x');
  uint64_t number = 0;
  if (xpos == std::string::npos && !isHex) try { number = std::stoull(s, 0, 10); } catch (...) {}
  else try { number = std::stoull(s, 0, 16); } catch (...) {}
  return number;
}

int32_t Math::getOctalNumber(const std::string &s) {
  int32_t number = 0;
  try { number = std::stoll(s, 0, 8); } catch (...) {}
  return number;
}

double Math::getDouble(const std::string &s) {
  double number = 0;
  try { number = std::stod(s); } catch (...) {}
  return number;
}

uint32_t Math::getIeee754Binary32(float value) {
  int32_t sign = 0;
  int32_t integer;
  int32_t exponent = 127;
  int32_t fraction = 0;

  if (value < 0) {
    sign = 0x80000000;
    value = -value;
  }

  integer = floor(value);
  value -= integer;

  for (int32_t i = 22; i >= 0; i--) {
    value += value;
    fraction += floor(value) * pow(2, i);
    value -= floor(value);
  }

  while ((integer != 1) && (exponent > 0) && (exponent < 255)) {
    if (integer > 1) {
      fraction = ((integer & 1) << 22) + (fraction >> 1);
      integer = integer >> 1;
      exponent++;
    } else {
      integer = (fraction & 0x400000) >> 22;
      fraction = (fraction & 0x3FFFFF) << 1;
      value += value;
      fraction += floor(value);
      value -= floor(value);
      exponent--;
    }
  }

  return sign + (exponent << 23) + fraction;
}

uint64_t Math::getIeee754Binary64(double value) {
  int64_t sign = 0;
  int64_t integer;
  int64_t exponent = 1023;
  int64_t fraction = 0;

  if (value < 0) {
    sign = 0x8000000000000000ll;
    value = -value;
  }

  integer = floor(value);
  value -= integer;

  for (int32_t i = 51; i >= 0; i--) {
    value += value;
    fraction += floor(value) * pow(2, i);
    value -= floor(value);
  }

  while ((integer != 1) && (exponent > 0) && (exponent < 2047)) {
    if (integer > 1) {
      fraction = ((integer & 1) << 51) + (fraction >> 1);
      integer = integer >> 1;
      exponent++;
    } else {
      integer = (fraction & 0x8000000000000ll) >> 51;
      fraction = (fraction & 0x7FFFFFFFFFFFFll) << 1;
      value += value;
      fraction += floor(value);
      value -= floor(value);
      exponent--;
    }
  }

  return sign + (exponent << 52) + fraction;
}

float Math::getFloatFromIeee754Binary32(uint32_t binary32) {
  int32_t sign = (binary32 & 0x80000000) ? -1 : 1;
  int32_t exponent = ((binary32 & 0x7F800000) >> 23) - 127;
  float fraction = (binary32 & 0x7FFFFF) + 0x800000;
  fraction = fraction / 0x800000;
  float result = sign * fraction * (float)pow(2, exponent);

  return result;
}

double Math::getDoubleFromIeee754Binary64(uint64_t binary64) {
  int64_t sign = (binary64 & 0x8000000000000000ll) ? -1 : 1;
  int64_t exponent = ((binary64 & 0x7FF0000000000000ll) >> 52) - 1023;
  double fraction = (binary64 & 0xFFFFFFFFFFFFFll) + 0x10000000000000ll;
  fraction = fraction / 0x10000000000000ll;
  double result = sign * fraction * (double)pow(2, exponent);

  return result;
}

std::string Math::toString(double number) {
  std::stringstream out;
  out << number;
  std::string string = out.str();
  auto pos = string.find('.');
  if (pos == std::string::npos) return string;
  int32_t decimalPlaces = string.size() - pos - 1;
  if (decimalPlaces > 3 && string[string.size() - 2] == string.back() && string[string.size() - 3] == string.back()) {
    double factor = std::pow(10, decimalPlaces - 1);
    //Round to 9 digits
    number = std::floor(number * factor + 0.5) / factor;
    out.str(std::string());
    out.clear();
    out << number;
  }
  return out.str();
}

std::string Math::toString(double number, int32_t precision) {
  std::ostringstream out;
  out << std::setiosflags(std::ios_base::fixed | std::ios_base::dec) << std::setprecision(precision) << number;
  return out.str();
}

int32_t Math::clamp(int32_t value, int32_t min, int32_t max) {
  if (value > max) return max;
  if (value < min) return min;
  return value;
}

double Math::clamp(double value, double min, double max) {
  if (value > max) return max;
  if (value < min) return min;
  return value;
}

int32_t Math::scale(int32_t value, int32_t valueMin, int32_t valueMax, int32_t scaleMin, int32_t scaleMax) {
  double vPerc = ((double)(value - valueMin)) / (valueMax - valueMin);
  double bigSpan = vPerc * (scaleMax - scaleMin);

  return std::lround(scaleMin + bigSpan);
}

double Math::scale(double value, double valueMin, double valueMax, double scaleMin, double scaleMax) {
  double vPerc = (value - valueMin) / (valueMax - valueMin);
  double bigSpan = vPerc * (scaleMax - scaleMin);

  return scaleMin + bigSpan;
}

double Math::metricExponentialMovingAverage(double interval, double period, double metric, double lastAverage) {
  return (1.0 - std::exp(-interval / period)) * metric + std::exp(-interval / period) * lastAverage;
}

}

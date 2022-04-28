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

#ifndef HOMEGEARMATH_H_
#define HOMEGEARMATH_H_

#include <string>
#include <map>
#include <cmath>
#include <bitset>

namespace BaseLib {
class Math {
 public:
  /**
   * Class defining a point in 2D space with numbers of type double.
   */
  class Point2D {
   public:
    Point2D() {}
    /**
     * Constructor converting a coordinate string with coordinates seperated by ";" (i. e. "0.3613;0.162").
     */
    Point2D(const std::string &s);
    Point2D(double x, double y) {
      this->x = x;
      this->y = y;
    }
    virtual ~Point2D() {}

    std::string toString() { return std::to_string(x) + ';' + std::to_string(y); }

    double x = 0;
    double y = 0;
  };

  typedef Point2D Vector2D;

  class Point3D {
   public:
    Point3D() {}
    /**
     * Constructor converting a coordinate string with coordinates seperated by ";" (i. e. "0.3613;0.162;0.543").
     */
    Point3D(const std::string &s);
    Point3D(double x, double y, double z) {
      this->x = x;
      this->y = y;
      this->z = z;
    }
    virtual ~Point3D() {}

    std::string toString() { return std::to_string(x) + ';' + std::to_string(y) + ';' + std::to_string(z); }

    double x = 0;
    double y = 0;
    double z = 0;
  };

  typedef Point3D Vector3D;

  /**
   * Class defining a 3x3 matrix.
   */
  class Matrix3x3 {
   public:
    double p00 = 0;
    double p10 = 0;
    double p20 = 0;
    double p01 = 0;
    double p11 = 0;
    double p21 = 0;
    double p02 = 0;
    double p12 = 0;
    double p22 = 0;

    Matrix3x3() {}

    virtual ~Matrix3x3() {}

    /**
     * Inverses the matrix.
     *
     * @param[out] inversedMatrix The inversed matrix;
     */
    void inverse(Matrix3x3 &inversedMatrix);

    Math::Vector3D operator*(const Math::Vector3D &v) const;

    double determinant();

    std::string toString();
  };

  /**
   * Class defining a line.
   */
  class Line {
   public:
    Line() {}
    Line(const Point2D &a, const Point2D &b) {
      _a = a;
      _b = b;
    }
    virtual ~Line() {}

    Point2D getA() const { return _a; }
    void setA(Point2D value) { _a = value; }
    void setA(Point2D &value) { _a = value; }
    Point2D getB() const { return _b; }
    void setB(Point2D value) { _b = value; }
    void setB(Point2D &value) { _b = value; }

    /**
     * Finds the closest point on the line to a point.
     *
     * @param[in] p The point to get the closest point on the line for.
     * @param[out] r The point on the line which is closest to p.
     */
    void closestPointToPoint(const Point2D &p, Point2D &r);
   protected:
    Point2D _a;
    Point2D _b;
  };

  /**
   * Class defining a triangle.
   */
  class Triangle {
   public:
    Triangle() {}
    Triangle(const Point2D &a, const Point2D &b, const Point2D &c) {
      _a = a;
      _b = b;
      _c = c;
    }
    virtual ~Triangle() {}

    Point2D getA() const { return _a; }
    void setA(Point2D value) { _a = value; }
    void setA(Point2D &value) { _a = value; }
    Point2D getB() const { return _b; }
    void setB(Point2D value) { _b = value; }
    void setB(Point2D &value) { _b = value; }
    Point2D getC() const { return _c; }
    void setC(Point2D value) { _c = value; }
    void setC(Point2D &value) { _c = value; }

    /**
     * Calculates the distance of a point to the triangle.
     *
     * @see <a href="http://wonderfl.net/c/b27F">Flash example by mutantleg</a>
     *
     * @param[in] p The point to check.
     * @param[out] closestPoint (default nullptr) When specified, the closest point in/on the triangle is stored in this variable.
     * @return Returns the squared distance or 0 if the point is within the triangle.
     */
    double distance(const Point2D &p, Point2D *closestPoint = nullptr);
   protected:
    Point2D _a;
    Point2D _b;
    Point2D _c;
  };

  Math() = delete;

  /**
   * Destructor.
   * Does nothing.
   */
  ~Math() = default;

  /**
   * Checks if a string is a number.
   *
   * @param s The string to check.
   * @param hex (default false) Set to "true" if s contains a hexadecimal number.
   * @return Returns true if the string is a decimal or hexadecimal number, otherwise false.
   */
  static bool isNumber(const std::string &s, bool hex = false);

  /**
   * Converts a string (decimal or hexadecimal) to an integer.
   *
   * @see getDouble()
   * @see getUnsignedNumber()
   * @see getUnsignedNumber64()
   * @see getNumber()
   * @see getNumber64()
   * @see getOctalNumber()
   * @param s The string to convert.
   * @param isHex Set this parameter to "true", if the string is hexadecimal (default false). If the string is prefixed with "0x", it is automatically detected as hexadecimal.
   * @return Returns the integer or "0" on error.
   */
  static int32_t getNumber(const std::string &s, bool isHex = false);

  /**
   * Converts a string (decimal or hexadecimal) to an 64-bit integer.
   *
   * @see getDouble()
   * @see getUnsignedNumber()
   * @see getUnsignedNumber64()
   * @see getNumber64()
   * @see getOctalNumber()
   * @param s The string to convert.
   * @param isHex Set this parameter to "true", if the string is hexadecimal (default false). If the string is prefixed with "0x", it is automatically detected as hexadecimal.
   * @return Returns the integer or "0" on error.
   */
  static int64_t getNumber64(const std::string &s, bool isHex = false);

  /**
   * Converts a octal string to an integer.
   *
   * @see getDouble()
   * @see getUnsignedNumber()
   * @see getUnsignedNumber64()
   * @see getNumber()
   * @see getNumber64()
   * @param s The string to convert.
   * @return Returns the integer or "0" on error.
   */
  static int32_t getOctalNumber(const std::string &s);

  /**
   * Converts a hexadecimal character to an integer.
   *
   * @see getDouble()
   * @see getUnsignedNumber()
   * @see getUnsignedNumber64()
   * @see getNumber64()
   * @see getOctalNumber()
   * @param hexChar The hexadecimal character.
   * @return Returns the integer or "0" on error.
   */
  static int32_t getNumber(char hexChar);

  /**
   * Converts a string (decimal or hexadecimal) to an unsigned integer.
   *
   * @see getDouble()
   * @see getNumber()
   * @see getNumber64()
   * @see getUnsignedNumber64()
   * @see getOctalNumber()
   * @param s The string to convert.
   * @param isHex Set this parameter to "true", if the string is hexadecimal (default false). If the string is prefixed with "0x", it is automatically detected as hexadecimal.
   * @return Returns the unsigned integer or "0" on error.
   */
  static uint32_t getUnsignedNumber(const std::string &s, bool isHex = false);

  /**
   * Converts a string (decimal or hexadecimal) to an unsigned integer.
   *
   * @see getDouble()
   * @see getNumber()
   * @see getNumber64()
   * @see getUnsignedNumber()
   * @see getOctalNumber()
   * @param s The string to convert.
   * @param isHex Set this parameter to "true", if the string is hexadecimal (default false). If the string is prefixed with "0x", it is automatically detected as hexadecimal.
   * @return Returns the unsigned integer or "0" on error.
   */
  static uint64_t getUnsignedNumber64(const std::string &s, bool isHex = false);

  /*
   * Converts a float value to it's IEEE 754 binary32 representation.
   *
   * @param value The float value to convert.
   * @return Returns a uint32_t with the converted value MSB first.
   */
  static uint32_t getIeee754Binary32(float value);

  /*
   * Converts a float value to it's IEEE 754 binary64 representation.
   *
   * @param value The float value to convert.
   * @return Returns a uint64_t with the converted value MSB first.
   */
  static uint64_t getIeee754Binary64(double value);

  /*
   * Converts a value in IEEE 754 binary32 representation to float data must be MSB first.
   *
   * @param binary32 The binary32 value to convert.
   * @return Returns the converted value as float.
   */
  static float getFloatFromIeee754Binary32(uint32_t binary32);

  /*
   * Converts a value in IEEE 754 binary64 representation to float data must be MSB first.
   *
   * @param binary64 The binary64 value to convert.
   * @return Returns the converted value as float.
   */
  static double getDoubleFromIeee754Binary64(uint64_t binary64);

  /**
   * Converts a string to double.
   *
   * @see getNumber()
   * @see getUnsignedNumber()
   * @param s The string to convert to double.
   * @return Returns the number or "0" if the conversion was not successful.
   */
  static double getDouble(const std::string &s);

  /**
   * Converts a double to string removing any trailing zeros.
   *
   * @param number The number to convert
   * @return Returns the number.
   */
  static std::string toString(double number);

  /**
   * Converts a double to string.
   *
   * @param number The number to convert
   * @param precision The precision
   * @return Returns the number.
   */
  static std::string toString(double number, int32_t precision);

  /**
   * Forces a value between 'min' and 'max'. If the value is larger than 'max' then it is set to 'max'. If the value is smaller than 'man' it is set to 'min'.
   *
   * @param value The number to clamp.
   * @param min The lower boundary.
   * @param max The upper boundary.
   * @return Returns the clamped number.
   */
  static int32_t clamp(int32_t value, int32_t min, int32_t max);

  /**
   * Forces a value between 'min' and 'max'. If the value is larger than 'max' then it is set to 'max'. If the value is smaller than 'man' it is set to 'min'.
   *
   * @param value The number to clamp.
   * @param min The lower boundary.
   * @param max The upper boundary.
   * @return Returns the clamped number.
   */
  static double clamp(double value, double min, double max);

  /**
   * Scales 'value' between 'valueMin' and 'valueMax' to 'scaleMin' and 'scaleMax'.
   *
   * @param value The value to scale.
   * @param valueMin The lowest possible value of 'value'.
   * @param valueMax The highest possible value of 'value'.
   * @param scaleMin The lowest value to scale 'value' to.
   * @param scaleMax The highest value to scale 'value' to.
   * @return Returns the number.
   */
  static int32_t scale(int32_t value, int32_t valueMin, int32_t valueMax, int32_t scaleMin, int32_t scaleMax);

  /**
   * Scales 'value' between 'valueMin' and 'valueMax' to 'scaleMin' and 'scaleMax'.
   *
   * @param value The value to scale.
   * @param valueMin The lowest possible value of 'value'.
   * @param valueMax The highest possible value of 'value'.
   * @param scaleMin The lowest value to scale 'value' to.
   * @param scaleMax The highest value to scale 'value' to.
   * @return Returns the number.
   */
  static double scale(double value, double valueMin, double valueMax, double scaleMin, double scaleMax);

  /**
   * Calculates powers to the base 10.
   */
  static inline double Pow10(int32_t exponent) {
    static const double e[] =
        {
            1e+0,
            1e+1, 1e+2, 1e+3, 1e+4, 1e+5, 1e+6, 1e+7, 1e+8, 1e+9, 1e+10, 1e+11, 1e+12, 1e+13, 1e+14, 1e+15, 1e+16, 1e+17, 1e+18, 1e+19, 1e+20,
            1e+21, 1e+22, 1e+23, 1e+24, 1e+25, 1e+26, 1e+27, 1e+28, 1e+29, 1e+30, 1e+31, 1e+32, 1e+33, 1e+34, 1e+35, 1e+36, 1e+37, 1e+38, 1e+39, 1e+40,
            1e+41, 1e+42, 1e+43, 1e+44, 1e+45, 1e+46, 1e+47, 1e+48, 1e+49, 1e+50, 1e+51, 1e+52, 1e+53, 1e+54, 1e+55, 1e+56, 1e+57, 1e+58, 1e+59, 1e+60,
            1e+61, 1e+62, 1e+63, 1e+64, 1e+65, 1e+66, 1e+67, 1e+68, 1e+69, 1e+70, 1e+71, 1e+72, 1e+73, 1e+74, 1e+75, 1e+76, 1e+77, 1e+78, 1e+79, 1e+80,
            1e+81, 1e+82, 1e+83, 1e+84, 1e+85, 1e+86, 1e+87, 1e+88, 1e+89, 1e+90, 1e+91, 1e+92, 1e+93, 1e+94, 1e+95, 1e+96, 1e+97, 1e+98, 1e+99, 1e+100,
            1e+101, 1e+102, 1e+103, 1e+104, 1e+105, 1e+106, 1e+107, 1e+108, 1e+109, 1e+110, 1e+111, 1e+112, 1e+113, 1e+114, 1e+115, 1e+116, 1e+117, 1e+118, 1e+119, 1e+120,
            1e+121, 1e+122, 1e+123, 1e+124, 1e+125, 1e+126, 1e+127, 1e+128, 1e+129, 1e+130, 1e+131, 1e+132, 1e+133, 1e+134, 1e+135, 1e+136, 1e+137, 1e+138, 1e+139, 1e+140,
            1e+141, 1e+142, 1e+143, 1e+144, 1e+145, 1e+146, 1e+147, 1e+148, 1e+149, 1e+150, 1e+151, 1e+152, 1e+153, 1e+154, 1e+155, 1e+156, 1e+157, 1e+158, 1e+159, 1e+160,
            1e+161, 1e+162, 1e+163, 1e+164, 1e+165, 1e+166, 1e+167, 1e+168, 1e+169, 1e+170, 1e+171, 1e+172, 1e+173, 1e+174, 1e+175, 1e+176, 1e+177, 1e+178, 1e+179, 1e+180,
            1e+181, 1e+182, 1e+183, 1e+184, 1e+185, 1e+186, 1e+187, 1e+188, 1e+189, 1e+190, 1e+191, 1e+192, 1e+193, 1e+194, 1e+195, 1e+196, 1e+197, 1e+198, 1e+199, 1e+200,
            1e+201, 1e+202, 1e+203, 1e+204, 1e+205, 1e+206, 1e+207, 1e+208, 1e+209, 1e+210, 1e+211, 1e+212, 1e+213, 1e+214, 1e+215, 1e+216, 1e+217, 1e+218, 1e+219, 1e+220,
            1e+221, 1e+222, 1e+223, 1e+224, 1e+225, 1e+226, 1e+227, 1e+228, 1e+229, 1e+230, 1e+231, 1e+232, 1e+233, 1e+234, 1e+235, 1e+236, 1e+237, 1e+238, 1e+239, 1e+240,
            1e+241, 1e+242, 1e+243, 1e+244, 1e+245, 1e+246, 1e+247, 1e+248, 1e+249, 1e+250, 1e+251, 1e+252, 1e+253, 1e+254, 1e+255, 1e+256, 1e+257, 1e+258, 1e+259, 1e+260,
            1e+261, 1e+262, 1e+263, 1e+264, 1e+265, 1e+266, 1e+267, 1e+268, 1e+269, 1e+270, 1e+271, 1e+272, 1e+273, 1e+274, 1e+275, 1e+276, 1e+277, 1e+278, 1e+279, 1e+280,
            1e+281, 1e+282, 1e+283, 1e+284, 1e+285, 1e+286, 1e+287, 1e+288, 1e+289, 1e+290, 1e+291, 1e+292, 1e+293, 1e+294, 1e+295, 1e+296, 1e+297, 1e+298, 1e+299, 1e+300,
            1e+301, 1e+302, 1e+303, 1e+304, 1e+305, 1e+306, 1e+307, 1e+308
        };
    return e[exponent];
  }

  /**
   * Calculates the exponential moving average per time for performance metrics.
   *
   * @param interval The time since the last measurements. The time unit doesn't matter as long as it is the same unit as for `period`.
   * @param period The time to average over (e. g. 60 seconds or 15 minutes). The time unit doesn't matter as long as it is the same as for `interval`.
   * @param metric The current performance measurement.
   * @param lastAverage The last output of this method. Pass `0` when this is the first call.
   * @return Returns the average over the given time period.
   */
  static double metricExponentialMovingAverage(double interval, double period, double metric, double lastAverage);
 protected:
  /**
   * Map to faster convert hexadecimal numbers.
   */
  static const std::array<int32_t, 23> _asciiToBinaryTable;
};

}

#endif

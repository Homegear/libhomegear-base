/* Copyright 2013-2016 Sathya Laufer
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

#ifndef _PID_H_
#define _PID_H_

namespace BaseLib
{

class Pid
{
private:
	double _dt;
	double _pMax;
	double _pMin;
	double _oMax;
	double _oMin;
	double _Kp;
	double _Ki;
	double _Kd;
	double _previousError;
	double _integral;

	double clamp(double value, double min, double max);
	double scale(double value, double valueMin, double valueMax, double scaleMin, double scaleMax);
public:
	// Kp -  proportional gain
	// Ki -  Integral gain
	// Kd -  derivative gain
	// dt -  loop interval time
	// pMax - maximum value of process variable
	// pMin - minimum value of process variable
	// oMax - maximum value of output variable
	// oMin - minimum value of output variable
	Pid(double dt, double pMax, double pMin, double oMax, double oMin, double Kp, double Ki, double Kd);

	// Returns the manipulated variable given a setpoint and current process value
	double calculate(double setpoint, double processVariable);
	virtual ~Pid();
};

}

#endif

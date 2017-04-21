/* Copyright 2013-2017 Sathya Laufer
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

#include <cmath>
#include "Pid.h"

namespace BaseLib
{

Pid::Pid( double dt, double pMax, double pMin, double oMax, double oMin, double Kp, double Ki, double Kd) :
	_dt(dt),
    _pMax(pMax),
    _pMin(pMin),
	_oMax(oMax),
    _oMin(oMin),
    _Kp(Kp),
	_Ki(Ki),
    _Kd(Kd),
    _previousError(0),
    _integral(0)
{
}

double Pid::clamp(double value, double min, double max)
{
	if (value > max) return max;
	if (value < min) return min;
	return value;
}

double Pid::scale(double value, double valueMin, double valueMax, double scaleMin, double scaleMax)
{
	double vPerc = (value - valueMin) / (valueMax - valueMin);
	double bigSpan = vPerc * (scaleMax - scaleMin);

	return scaleMin + bigSpan;
}

double Pid::calculate(double setpoint, double processVariable)
{
	/*  Pseudocode from Wikipedia:
		previous_error = 0
		integral = 0
		start:
			error = setpoint - measured_value
			integral = integral + error*dt
			derivative = (error - previous_error)/dt
			output = Kp*error + Ki*integral + Kd*derivative
			previous_error = error
			wait(dt)
			goto start
	*/

	processVariable = clamp(processVariable, _pMin, _pMax);
	processVariable = scale(processVariable, _pMin, _pMax, -1.0f, 1.0f);

	setpoint = clamp(setpoint, _pMin, _pMax);
	setpoint = scale(setpoint, _pMin, _pMax, -1.0f, 1.0f);

	// {{{ PID
		double error = setpoint - processVariable;
		_integral += error * _dt;
		double derivative = (error - _previousError) / _dt;
		double output = _Kp * error + _Ki * _integral + _Kd * derivative;
		_previousError = error;
    // }}}

	output = clamp(output, -1.0f, 1.0f);
	output = scale(output, -1.0f, 1.0f, _oMin, _oMax);

	return output;
}

Pid::~Pid()
{
}

}

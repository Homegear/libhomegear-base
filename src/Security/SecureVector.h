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

#ifndef LIBHOMEGEAR_BASE_SECUREVECTOR_H
#define LIBHOMEGEAR_BASE_SECUREVECTOR_H

#include <vector>

namespace BaseLib
{
namespace Security
{

/**
 * The class only makes sure that the vector is not copyable and the data is zeroed on destruction. Once created, only
 * use methods prepended with the word "secure". You can safely use the methods secureAppend() and securePrepend().
 */
template<typename T>
class SecureVector : public std::vector<T>
{
public:
    SecureVector() : std::vector<T>() {};
    explicit SecureVector(size_t count) : std::vector<T>(count) {}
    explicit SecureVector(size_t count, const T& value) : std::vector<T>(count, value) {}
    SecureVector(const SecureVector&) = delete; //Copy constructor
    SecureVector(SecureVector&&) noexcept = default; //Move constructor
    SecureVector& operator=(const SecureVector&) = delete; //Copy assignment operator

    ~SecureVector()
    {
        std::fill(this->begin(), this->end(), 0);
    }

    void secureResize(size_t count)
    {
        if(count <= this->capacity())
        {
            this->resize(count);
            return;
        }

        std::vector<uint8_t> newVector;
        newVector.resize(count);
        std::copy(this->begin(), this->end(), newVector.begin());
        this->swap(newVector);
        std::fill(newVector.begin(), newVector.end(), 0);
    }

    void secureResize(size_t count, const T& value)
    {
        if(count <= this->capacity())
        {
            this->resize(count, value);
            return;
        }

        std::vector<uint8_t> newVector;
        newVector.resize(count, value);
        std::copy(this->begin(), this->end(), newVector.begin());
        this->swap(newVector);
        std::fill(newVector.begin(), newVector.end(), 0);
    }

    void securePrepend(const SecureVector& other)
    {
        std::vector<uint8_t> newVector;
        newVector.resize(this->size() + other.size());
        std::copy(other.begin(), other.end(), newVector.begin());
        std::copy(this->begin(), this->end(), newVector.begin() + other.size());
        this->swap(newVector);
        std::fill(newVector.begin(), newVector.end(), 0);
    }

    void secureAppend(const SecureVector& other)
    {
        std::vector<uint8_t> newVector;
        newVector.resize(this->size() + other.size());
        std::copy(this->begin(), this->end(), newVector.begin());
        std::copy(other.begin(), other.end(), newVector.begin() + this->size());
        this->swap(newVector);
        std::fill(newVector.begin(), newVector.end(), 0);
    }
};

}
}

#endif //LIBHOMEGEAR_BASE_SECUREVECTOR_H

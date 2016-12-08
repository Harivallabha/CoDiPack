/*
 * CoDiPack, a Code Differentiation Package
 *
 * Copyright (C) 2015 Chair for Scientific Computing (SciComp), TU Kaiserslautern
 * Homepage: http://www.scicomp.uni-kl.de
 * Contact:  Prof. Nicolas R. Gauger (codi@scicomp.uni-kl.de)
 *
 * Lead developers: Max Sagebaum, Tim Albring (SciComp, TU Kaiserslautern)
 *
 * This file is part of CoDiPack (http://www.scicomp.uni-kl.de/software/codi).
 *
 * CoDiPack is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 *
 * CoDiPack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU
 * General Public License along with CoDiPack.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: Max Sagebaum, Tim Albring, (SciComp, TU Kaiserslautern)
 */

#pragma once

#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <string>

/**
 * @brief Global namespace for CoDiPack - Code Differentiation Package
 */
namespace codi {

  enum struct IoError {
    Mode,
    Open,
    Write,
    Read
  };

  struct IoException {
      std::string text;
      IoError id;

      IoException(IoError id, const std::string& text, bool appendErrno) :
        text(text),
        id(id)
      {
        if(appendErrno) {
          this->text += " (Internal error: ";
          this->text += strerror(errno);
          this->text += ")";
        }
      }
  };

  class CoDiIoHandle {

      FILE* fileHandle;
      bool writeMode;

    public:

      CoDiIoHandle(const std::string& file, bool write) {
        writeMode = write;
        fileHandle = NULL;

        if(write) {
          fileHandle = fopen(file.c_str(), "wb");
        } else {
          fileHandle = fopen(file.c_str(), "rb");
        }

        if(NULL == fileHandle) {
          throw IoException(IoError::Open , "Could not open file: " + file, true);
        }
      }

      ~CoDiIoHandle() {
        if(NULL != fileHandle) {
          fclose(fileHandle);
        }
      }

      template<typename Data>
      void writeData(Data* data, size_t length) {
        if(writeMode) {
          size_t s = fwrite(data, sizeof(Data), length, fileHandle);

          if(s != length) {
            throw IoException(IoError::Read, "Wrong number of bytes written.", true);
          }
        } else {
          throw IoException(IoError::Mode, "Using write io handle in wrong mode.", false);
        }
      }

      template<typename Data>
      void readData(Data* data, size_t length) {
        if(!writeMode) {
          size_t s = fread(data, sizeof(Data), length, fileHandle);

          if(s != length) {
            throw IoException(IoError::Read, "Wrong number of bytes read.", false);
          }
        } else {
          throw IoException(IoError::Mode, "Using read io handle in wrong mode.", false);
        }
      }
  };
}

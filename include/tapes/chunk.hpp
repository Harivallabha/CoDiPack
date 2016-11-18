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

#include <cstddef>
#include <new>
#include <string.h>

#include "../configure.h"
#include "../tools/io.hpp"

/**
 * @brief Global namespace for CoDiPack - Code Differentiation Package
 */
namespace codi {

  /**
   * @brief The basic interface for data chunks.
   *
   * The chunk interface is the basic interface all chunks have to implement.
   * The interface already provides the basic facilities for the
   * size of the data inside the chunk.
   */
  struct ChunkInterface {

    size_t size; /**< Size of the allocated data */
    size_t usedSize; /**< Number of used items in the data array */

    /**
     * @brief Create a chunk with the given size.
     * @param size  The size of the data in the chunk.
     */
    explicit ChunkInterface(const size_t& size) :
      size(size),
      usedSize(0) {}

    virtual ~ChunkInterface() {}

    virtual void writeData(CoDiIoHandle& handle) = 0;

    virtual void readData(CoDiIoHandle& handle) = 0;

    virtual void allocateData() = 0;

    virtual void deleteData() = 0;

    /**
     * @brief Get the maximum size of the chunk
     * @return The maximum number of items.
     */
    CODI_INLINE size_t getSize() const {
      return size;
    }

    /**
     * @brief Get the number of used items.
     * @return The number of used items.
     */
    CODI_INLINE size_t getUsedSize() const {
      return usedSize;
    }

    /**
     * @brief Get the number of free items.
     * @return The number of free items.
     */
    CODI_INLINE size_t getUnusedSize() const {
      return size - usedSize;
    }

    /**
     * @brief Fully reset the data in this chunk.
     */
    CODI_INLINE void reset() {
      usedSize = 0;
    }

    /**
     * @brief Set the number of used items in this chunk.
     * @param usage   The number of used items.
     */
    CODI_INLINE void setUsedSize(const size_t& usage) {
      usedSize = usage;
    }

    /**
     * @brief Store the data of the chunk.
     *
     * This method is called when the data of a chunk is no
     * longer directly needed and can be stored somewhere else.
     */
    CODI_INLINE void store() {}

    /**
     * @brief Load the data of the chunk.
     *
     * This method is called when the data of a chunk is needed by the
     * evaluation process.
     */
    CODI_INLINE void load() {}
  };

  /**
   * @brief Chunk with one data array.
   *
   * This chunk contains one data array which is stored in memory.
   *
   * @tparam Data   The type of the stored data.
   */
  template<typename Data>
  struct Chunk1 final : public ChunkInterface {

    /**
     * @brief The combined size of one entry in all data arrays.
     */
    const static size_t EntrySize = sizeof(Data);

    Data* data; /**< The data of the chunk */

    /**
     * @brief Creates the data of the chunk.
     *
     * @param size The size of the data in the chunk.
     */
    Chunk1(const size_t& size) : ChunkInterface(size) {
      allocateData();
    }

    /**
     * @brief Deletes the data array
     */
    ~Chunk1() {
      deleteData();
    }

    void writeData(CoDiIoHandle& handle) {
      handle.writeData(data, size);
    }

    void readData(CoDiIoHandle& handle) {
      if(NULL == data) {
        allocateData();
      }
      handle.readData(data, size);
    }

    void allocateData() {
      data = new Data[size];
    }

    void deleteData() {
      if(NULL != data) {
        delete [] data;
        data = NULL;
      }
    }

    /**
     * @brief Set the size of the array.
     * @param size  The new size of the array.
     */
    void resize(const size_t &size) {
      this->~Chunk1();
      new (this) Chunk1(size);
    }

    /**
     * @brief Set the data values to the current position and increment the used size.
     * @param value  The value which are set to the data.
     */
    CODI_INLINE void setDataAndMove(const Data& value) {
      codiAssert(getUnusedSize() != 0);
      data[usedSize] = value;
      ++usedSize;
    }

    /**
     * @brief Returns a pointer to the data array at the given position.
     * @param   index  The index in the data array.
     * @param pointer  Pointer that is set to the internal data pointer.
     */
    CODI_INLINE void dataPointer(const size_t& index, Data* &pointer) {
      codiAssert(index <= ChunkInterface::size);

      pointer = &data[index];
    }
  };

  /**
   * @brief Chunk with two data arrays.
   *
   * This chunk contains two data arrays which are stored in memory.
   *
   * @tparam Data1   The first type of the stored data.
   * @tparam Data2   The second type of the stored data.
   */
  template<typename Data1, typename Data2>
  struct Chunk2 final : public ChunkInterface {

    /**
     * @brief The combined size of one entry in all data arrays.
     */
    const static size_t EntrySize = sizeof(Data1) + sizeof(Data2);

    Data1* data1; /**< First data item of the chunk */
    Data2* data2; /**< Second data item of the chunk */

    /**
     * @brief Creates the data of the chunk.
     *
     * @param size The size of the data in the chunk.
     */
    Chunk2(const size_t& size) : ChunkInterface(size) {
      allocateData();
    }

    /**
     * @brief Deletes the data arrays
     */
    ~Chunk2() {
      deleteData();
    }

    void writeData(CoDiIoHandle& handle) {
      handle.writeData(data1, size);
      handle.writeData(data2, size);
    }

    void readData(CoDiIoHandle& handle) {
      if(NULL == data1) {
        allocateData();
      }
      handle.readData(data1, size);
      handle.readData(data2, size);
    }

    void allocateData() {
      data1 = new Data1[size];
      data2 = new Data2[size];
    }

    void deleteData() {
      if(NULL != data1) {
        delete [] data1;
        data1 = NULL;
      }

      if(NULL != data2) {
        delete [] data2;
        data2 = NULL;
      }
    }

    /**
     * @brief Set the size of the arrays.
     * @param size  The new size of the arrays.
     */
    void resize(const size_t &size) {
      this->~Chunk2();
      new (this) Chunk2(size);
    }

    /**
     * @brief Set the data values to the current position and increment the used size.
     * @param value1  The value for the first data array.
     * @param value2  The value for the second data array.
     */
    CODI_INLINE void setDataAndMove(const Data1& value1, const Data2& value2) {
      codiAssert(getUnusedSize() != 0);
      data1[usedSize] = value1;
      data2[usedSize] = value2;
      ++usedSize;
    }

    /**
     * @brief Returns a pointer to the data array at the given position.
     * @param    index  The index in the data array.
     * @param pointer1  Pointer that is set to the internal data pointer of the first array.
     * @param pointer2  Pointer that is set to the internal data pointer of the second array.
     * @return A pointer to the data.
     */
    CODI_INLINE void dataPointer(const size_t& index, Data1* &pointer1, Data2* &pointer2) {
      codiAssert(index <= ChunkInterface::size);
      pointer1 = &data1[index];
      pointer2 = &data2[index];
    }
  };

  /**
   * @brief Chunk with three data arrays.
   *
   * This chunk contains three data arrays which are stored in memory.
   *
   * @tparam Data1   The first type of the stored data.
   * @tparam Data2   The second type of the stored data.
   * @tparam Data3   The third type of the stored data.
   */
  template<typename Data1, typename Data2, typename Data3>
  struct Chunk3 final : public ChunkInterface {

    /**
     * @brief The combined size of one entry in all data arrays.
     */
    const static size_t EntrySize = sizeof(Data1) + sizeof(Data2) + sizeof(Data3);

    Data1* data1; /**< First data item of the chunk */
    Data2* data2; /**< Second data item of the chunk */
    Data3* data3; /**< Third data item of the chunk */

    /**
     * @brief Creates the data of the chunk.
     *
     * @param size The size of the data in the chunk.
     */
    Chunk3(const size_t& size) : ChunkInterface(size) {
      allocateData();
    }

    /**
     * @brief Deletes the data arrays
     */
    ~Chunk3() {
      deleteData();
    }

    void writeData(CoDiIoHandle& handle) {
      handle.writeData(data1, size);
      handle.writeData(data2, size);
      handle.writeData(data3, size);
    }

    void readData(CoDiIoHandle& handle) {
      if(NULL == data1) {
        allocateData();
      }
      handle.readData(data1, size);
      handle.readData(data2, size);
      handle.readData(data3, size);
    }

    void allocateData() {
      data1 = new Data1[size];
      data2 = new Data2[size];
      data3 = new Data3[size];
    }

    void deleteData() {
      if(NULL != data1) {
        delete [] data1;
        data1 = NULL;
      }

      if(NULL != data2) {
        delete [] data2;
        data2 = NULL;
      }

      if(NULL != data3) {
        delete [] data3;
        data3 = NULL;
      }
    }

    /**
     * @brief Set the size of the arrays.
     * @param size  The new size of the arrays.
     */
    void resize(const size_t &size) {
      this->~Chunk3();
      new (this) Chunk3(size);
    }

    /**
     * @brief Set the data values to the current position and increment the used size.
     * @param value1  The value for the first data array.
     * @param value2  The value for the second data array.
     * @param value3  The value for the third data array.
     */
    CODI_INLINE void setDataAndMove(const Data1& value1, const Data2& value2, const Data3& value3) {
      codiAssert(getUnusedSize() != 0);
      data1[usedSize] = value1;
      data2[usedSize] = value2;
      data3[usedSize] = value3;
      ++usedSize;
    }

    /**
     * @brief Returns a pointer to the data array at the given position.
     * @param    index  The index in the data array.
     * @param pointer1  Pointer that is set to the internal data pointer of the first array.
     * @param pointer2  Pointer that is set to the internal data pointer of the second array.
     * @param pointer3  Pointer that is set to the internal data pointer of the third array.
     * @return A pointer to the data.
     */
    CODI_INLINE void dataPointer(const size_t& index, Data1* &pointer1, Data2* &pointer2, Data3* &pointer3) {
      codiAssert(index <= ChunkInterface::size);
      pointer1 = &data1[index];
      pointer2 = &data2[index];
      pointer3 = &data3[index];
    }
  };

  /**
   * @brief Chunk with four data arrays.
   *
   * This chunk contains four data arrays which are stored in memory.
   *
   * @tparam Data1   The first type of the stored data.
   * @tparam Data2   The second type of the stored data.
   * @tparam Data3   The third type of the stored data.
   * @tparam Data4   The fourth type of the stored data.
   */
  template<typename Data1, typename Data2, typename Data3, typename Data4>
  struct Chunk4 final : public ChunkInterface {

    /**
     * @brief The combined size of one entry in all data arrays.
     */
    const static size_t EntrySize = sizeof(Data1) + sizeof(Data2) + sizeof(Data3) + sizeof(Data4);

    Data1* data1; /**< First data item of the chunk */
    Data2* data2; /**< Second data item of the chunk */
    Data3* data3; /**< Third data item of the chunk */
    Data4* data4; /**< Fourth data item of the chunk */

    /**
     * @brief Creates the data of the chunk.
     *
     * @param size The size of the data in the chunk.
     */
    Chunk4(const size_t& size) : ChunkInterface(size) {
      allocateData();
    }

    /**
     * @brief Deletes the data arrays
     */
    ~Chunk4() {
      deleteData();
    }

    void writeData(CoDiIoHandle& handle) {
      handle.writeData(data1, size);
      handle.writeData(data2, size);
      handle.writeData(data3, size);
      handle.writeData(data4, size);
    }

    void readData(CoDiIoHandle& handle) {
      if(NULL == data1) {
        allocateData();
      }
      handle.readData(data1, size);
      handle.readData(data2, size);
      handle.readData(data3, size);
      handle.readData(data4, size);
    }

    void allocateData() {
      data1 = new Data1[size];
      data2 = new Data2[size];
      data3 = new Data3[size];
      data4 = new Data4[size];
    }

    void deleteData() {
      if(NULL != data1) {
        delete [] data1;
        data1 = NULL;
      }

      if(NULL != data2) {
        delete [] data2;
        data2 = NULL;
      }

      if(NULL != data3) {
        delete [] data3;
        data3 = NULL;
      }

      if(NULL != data4) {
        delete [] data4;
        data4 = NULL;
      }
    }

    /**
     * @brief Set the size of the arrays.
     * @param size  The new size of the arrays.
     */
    void resize(const size_t &size) {
      this->~Chunk4();
      new (this) Chunk4(size);
    }

    /**
     * @brief Set the data values to the current position and increment the used size.
     * @param value1  The value for the first data array.
     * @param value2  The value for the second data array.
     * @param value3  The value for the third data array.
     * @param value4  The value for the fourth data array.
     */
    CODI_INLINE void setDataAndMove(const Data1& value1, const Data2& value2, const Data3& value3, const Data4& value4) {
      codiAssert(getUnusedSize() != 0);
      data1[usedSize] = value1;
      data2[usedSize] = value2;
      data3[usedSize] = value3;
      data4[usedSize] = value4;
      ++usedSize;
    }

    /**
     * @brief Returns a pointer to the data array at the given position.
     * @param    index  The index in the data array.
     * @param pointer1  Pointer that is set to the internal data pointer of the first array.
     * @param pointer2  Pointer that is set to the internal data pointer of the second array.
     * @param pointer3  Pointer that is set to the internal data pointer of the third array.
     * @param pointer4  Pointer that is set to the internal data pointer of the fourth array.
     * @return A pointer to the data.
     */
    CODI_INLINE void dataPointer(const size_t& index, Data1* &pointer1, Data2* &pointer2, Data3* &pointer3, Data4* &pointer4) {
      codiAssert(index <= ChunkInterface::size);
      pointer1 = &data1[index];
      pointer2 = &data2[index];
      pointer3 = &data3[index];
      pointer4 = &data4[index];
    }
  };

}

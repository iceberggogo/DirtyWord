//
//  dart.h
//  HoolaiEngine
//
//  Created by liji  on 14-1-18.
//  Copyright (c) 2014年 Hoolai. All rights reserved.
//

#ifndef DARTS_H_
#define DARTS_H_

#include <vector>
#include <cstring>
#include <cstdio>

namespace Darts {

	template <class T> inline T _max(T x, T y) { return(x > y) ? x : y; }
	template <class T> inline T* _resize(T* ptr, size_t n, size_t l, T v) {
		T *tmp = new T[l];
		for (size_t i = 0; i < n; ++i) tmp[i] = ptr[i];
		for (size_t i = n; i < l; ++i) tmp[i] = v;
		delete [] ptr;
		return tmp;
	}

	template <class T>
	class Length {
	public: size_t operator()(const T *key) const
			{ size_t i; for (i = 0; key[i] != static_cast<T>(0); ++i) {} return i; }
	};

	template <> class Length<char> {
	public: size_t operator()(const char *key) const
			{ return std::strlen(key); }
	};

	template  <class node_type_,  class node_u_type_,
        class array_type_, class array_u_type_,
        class length_func_ = Length<node_type_> >
        class DoubleArrayImpl {
	public:
		typedef array_type_  value_type;
		typedef node_type_   key_type;
		typedef array_type_  result_type;  // for compatibility

		struct result_pair_type {
			value_type value;
			size_t     length;
		};

		explicit DoubleArrayImpl(): array_(0), used_(0),
			size_(0), alloc_size_(0),
			no_delete_(0), error_(0) {}
		virtual ~DoubleArrayImpl() { clear(); }

		void set_result(value_type *x, value_type r, size_t) const {
			*x = r;
		}

		void set_result(result_pair_type *x, value_type r, size_t l) const {
			x->value = r;
			x->length = l;
		}

		void set_array(void *ptr, size_t size = 0) {
			clear();
			array_ = reinterpret_cast<unit_t *>(ptr);
			no_delete_ = true;
			size_ = size;
		}

		const void *array() const {
			return const_cast<const void *>(reinterpret_cast<void *>(array_));
		}

		void clear() {
			if (!no_delete_)
				delete [] array_;
			delete [] used_;
			array_ = 0;
			used_ = 0;
			alloc_size_ = 0;
			size_ = 0;
			no_delete_ = false;
		}

		size_t unit_size()  const { return sizeof(unit_t); }
		size_t size()       const { return size_; }
		size_t total_size() const { return size_ * sizeof(unit_t); }

		size_t nonzero_size() const {
			size_t result = 0;
			for (size_t i = 0; i < size_; ++i)
				if (array_[i].check) ++result;
			return result;
		}

		int open(const char *file, size_t size = 0)
        {
				std::FILE *fp = std::fopen(file, "rb");
				if (!fp) return -1;
				if (std::fseek(fp, 0, SEEK_SET) != 0) return -1;

				if (!size) {
					if (std::fseek(fp, 0L, SEEK_END) != 0) return -1;
					size = std::ftell(fp);
                    if (std::fseek(fp, 0, SEEK_SET) != 0) return -1;
				}

				clear();

				size_ = size;
				size_ /= sizeof(unit_t);
				array_ = new unit_t[size_];
            if (size_ != std::fread(reinterpret_cast<unit_t *>(array_),
                                    sizeof(unit_t), size_, fp))
                    return -1;
				std::fclose(fp);

				return 0;
            
            
//            hoolai::HLFileData* fp = hoolai::HLResourceManager::getSingleton()->getFileData(file);
//            if (!fp || fp->size == 0)
//            {
//                return -1;
//            }
//            size_ = fp->size;
//            size_ /= sizeof(unit_t);
//            array_ = new unit_t[size_];
//            fp->read(array_, sizeof(unit_t)*size_);
//            
//            delete fp;
//            return 0;
		}

		template <class T>
		inline void exactMatchSearch(const key_type *key,
			T & result,
			size_t len = 0,
			size_t node_pos = 0) const {
				result = exactMatchSearch<T> (key, len, node_pos);
				return;
		}

		template <class T>
		inline T exactMatchSearch(const key_type *key,
			size_t len = 0,
			size_t node_pos = 0) const {
				if (!len) len = length_func_()(key);

				T result;
				set_result(&result, -1, 0);

				register array_type_  b = array_[node_pos].base;
				register array_u_type_ p;

				for (register size_t i = 0; i < len; ++i) {
					p = b +(node_u_type_)(key[i]) + 1;
					if (static_cast<array_u_type_>(b) == array_[p].check)
						b = array_[p].base;
					else
						return result;
				}

				p = b;
				array_type_ n = array_[p].base;
				if (static_cast<array_u_type_>(b) == array_[p].check && n < 0)
					set_result(&result, -n-1, len);

				return result;
		}

		template <class T>
		size_t commonPrefixSearch(const key_type *key,
			T* result,
			size_t result_len,
			size_t len = 0,
			size_t node_pos = 0) const {
				if (!len) len = length_func_()(key);

				register array_type_  b   = array_[node_pos].base;
				register size_t     num = 0;
				register array_type_  n;
				register array_u_type_ p;

				for (register size_t i = 0; i < len; ++i) {
					p = b;  // + 0;
					n = array_[p].base;
					if ((array_u_type_) b == array_[p].check && n < 0) {
						// result[num] = -n-1;
						if (num < result_len) set_result(&result[num], -n-1, i);
						++num;
					}

					p = b +(node_u_type_)(key[i]) + 1;
					if ((array_u_type_) b == array_[p].check)
						b = array_[p].base;
					else
						return num;
				}

				p = b;
				n = array_[p].base;

				if ((array_u_type_)b == array_[p].check && n < 0) {
					if (num < result_len) set_result(&result[num], -n-1, len);
					++num;
				}

				return num;
		}

		value_type traverse(const key_type *key,
			size_t &node_pos,
			size_t &key_pos,
			size_t len = 0) const {
				if (!len) len = length_func_()(key);

				register array_type_  b = array_[node_pos].base;
				register array_u_type_ p;

				for (; key_pos < len; ++key_pos) {
					p = b + (node_u_type_)(key[key_pos]) + 1;
					if (static_cast<array_u_type_>(b) == array_[p].check) {
						node_pos = p;
						b = array_[p].base;
					} else {
						return -2;  // no node
					}
				}

				p = b;
				array_type_ n = array_[p].base;
				if (static_cast<array_u_type_>(b) == array_[p].check && n < 0)
					return -n-1;

				return -1;  // found, but no value
		}

	private:
		struct node_t {
			array_u_type_ code;
			size_t     depth;
			size_t     left;
			size_t     right;
		};

		struct unit_t {
			array_type_   base;
			array_u_type_ check;
		};

		unit_t        *array_;
		unsigned char *used_;
		size_t        size_;
		size_t        alloc_size_;
		size_t        key_size_;
		const node_type_    **key_;
		const size_t        *length_;
		const array_type_   *value_;
		size_t        progress_;
		size_t        next_check_pos_;
		bool          no_delete_;
		int           error_;
		int (*progress_func_)(size_t, size_t);

		size_t resize(const size_t new_size) {
			unit_t tmp;
			tmp.base = 0;
			tmp.check = 0;
			array_ = _resize(array_, alloc_size_, new_size, tmp);
			used_  = _resize(used_, alloc_size_, new_size,
				static_cast<unsigned char>(0));
			alloc_size_ = new_size;
			return new_size;
		}

		size_t fetch(const node_t &parent, std::vector <node_t> &siblings) {
			if (error_ < 0) return 0;

			array_u_type_ prev = 0;

			for (size_t i = parent.left; i < parent.right; ++i) {
				if ((length_ ? length_[i] : length_func_()(key_[i])) < parent.depth)
					continue;

				const node_u_type_ *tmp = reinterpret_cast<const node_u_type_ *>(key_[i]);

				array_u_type_ cur = 0;
				if ((length_ ? length_[i] : length_func_()(key_[i])) != parent.depth)
					cur = (array_u_type_)tmp[parent.depth] + 1;

				if (prev > cur) {
					error_ = -3;
					return 0;
				}

				if (cur != prev || siblings.empty()) {
					node_t tmp_node;
					tmp_node.depth = parent.depth + 1;
					tmp_node.code  = cur;
					tmp_node.left  = i;
					if (!siblings.empty()) siblings[siblings.size()-1].right = i;

					siblings.push_back(tmp_node);
				}

				prev = cur;
			}

			if (!siblings.empty())
				siblings[siblings.size()-1].right = parent.right;

			return siblings.size();
		}

		size_t insert(const std::vector <node_t> &siblings) {
			if (error_ < 0) return 0;

			size_t begin = 0;
			size_t pos   = _max((size_t)siblings[0].code + 1, next_check_pos_) - 1;
			size_t nonzero_num = 0;
			int    first = 0;

			if (alloc_size_ <= pos) resize(pos + 1);

			while (true) {
next:
				++pos;

				if (alloc_size_ <= pos) resize(pos + 1);

				if (array_[pos].check) {
					++nonzero_num;
					continue;
				} else if (!first) {
					next_check_pos_ = pos;
					first = 1;
				}

				begin = pos - siblings[0].code;
				if (alloc_size_ <= (begin + siblings[siblings.size()-1].code))
					resize(static_cast<size_t>(alloc_size_ *
					_max(1.05, 1.0 * key_size_ / progress_)));

				if (used_[begin]) continue;

				for (size_t i = 1; i < siblings.size(); ++i)
					if (array_[begin + siblings[i].code].check != 0) goto next;

				break;
			}

			// -- Simple heuristics --
			// if the percentage of non-empty contents in check between the index
			// 'next_check_pos' and 'check' is greater than some constant
			// value(e.g. 0.9),
			// new 'next_check_pos' index is written by 'check'.
			if (1.0 * nonzero_num/(pos - next_check_pos_ + 1) >= 0.95)
				next_check_pos_ = pos;

			used_[begin] = 1;
			size_ = _max(size_,
				begin +
				static_cast<size_t>(siblings[siblings.size() - 1].code + 1));

			for (size_t i = 0; i < siblings.size(); ++i)
				array_[begin + siblings[i].code].check = begin;

			for (size_t i = 0; i < siblings.size(); ++i) {
				std::vector <node_t> new_siblings;

				if (!fetch(siblings[i], new_siblings)) {
					array_[begin + siblings[i].code].base =
						value_ ?
						static_cast<array_type_>(-value_[siblings[i].left]-1) :
					static_cast<array_type_>(-siblings[i].left-1);

					if (value_ && (array_type_)(-value_[siblings[i].left]-1) >= 0) {
						error_ = -2;
						return 0;
					}

					++progress_;
					if (progress_func_)(*progress_func_)(progress_, key_size_);

				} else {
					size_t h = insert(new_siblings);
					array_[begin + siblings[i].code].base = h;
				}
			}

			return begin;
		}

	};

#if 4 == 2
	typedef Darts::DoubleArrayImpl<char, unsigned char, short,
		unsigned short> DoubleArray;
#define DARTS_ARRAY_SIZE_IS_DEFINED 1
#endif

#if 4 == 4 && !defined(DARTS_ARRAY_SIZE_IS_DEFINED)
	typedef Darts::DoubleArrayImpl<char, unsigned char, int,
		unsigned int> DoubleArray;
#define DARTS_ARRAY_SIZE_IS_DEFINED 1
#endif

#if 4 == 4 && !defined(DARTS_ARRAY_SIZE_IS_DEFINED)
	typedef Darts::DoubleArrayImpl<char, unsigned char, long,
		unsigned long> DoubleArray;
#define DARTS_ARRAY_SIZE_IS_DEFINED 1
#endif

#if 4 == 8 && !defined(DARTS_ARRAY_SIZE_IS_DEFINED)
	typedef Darts::DoubleArrayImpl<char, unsigned char, long long,
		unsigned long long> DoubleArray;
#endif
}
#endif
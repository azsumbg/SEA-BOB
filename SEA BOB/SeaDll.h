#pragma once

#ifdef SEADLL_EXPORTS
#define SEADLL_API __declspec(dllexport)
#else
#define SEADLL_API __declspec(dllimport)
#endif

#include <random>

constexpr float scr_width{ 1000.0f };
constexpr float scr_height{ 800.0f };

constexpr float sky{ 50.0f };
constexpr float ground{ 750.0f };

enum class types {
	evil1 = 0, evil2 = 1, evil3 = 2, evil4 = 3, evil5 = 4,
	evil6 = 5, evil7 = 6, evil8 = 7, hero = 8, jelly = 9, field = 10, no_type = 11
};

enum class dirs { stop = 0, left = 1, right = 2, up = 3, down = 4 };

enum class states { move = 0, patrol = 1, stop = 2 };

struct FPOINT
{
	float x{ 0 };
	float y{ 0 };
};

namespace dll
{
	class SEADLL_API RANDIt
	{
	private:
		std::mt19937* twister{ nullptr };

	public:
		RANDIt();
		~RANDIt();

		int operator()(int min, int max);
	};

	template<typename T> class SEADLL_API PACK
	{
	private:
		T* m_ptr{ nullptr };
		size_t max_size{ 0 };
		size_t next_pos{ 0 };

		bool is_valid{ false };

	public:
		PACK() :max_size{ 1 }, m_ptr{ reinterpret_cast<T*>(calloc(1,sizeof(T))) }
		{
			if (m_ptr)is_valid = true;
		}
		PACK(size_t lenght) :max_size{ lenght }, m_ptr{ reinterpret_cast<T*>(calloc(max_size,sizeof(T))) }
		{
			if (m_ptr)is_valid = true;
		}
		~PACK()
		{
			if (m_ptr)free(m_ptr);
		}

		bool valid() const
		{
			return is_valid;
		}
		size_t capacity() const
		{
			return max_size;
		}
		size_t size() const
		{
			return next_pos;
		}

		void push_back(T element)
		{
			if (!is_valid)return;

			if (next_pos < max_size)
			{
				m_ptr[next_pos] = element;
				++next_pos;
				return;
			}
			else
			{
				m_ptr = reinterpret_cast<T*>(realloc(m_ptr, (max_size + 1) * sizeof(T)));
				m_ptr[next_pos] = element;

				++max_size;
				++next_pos;
				return;
			}
		}
		void push_front(T element)
		{
			if (!is_valid)return;
			*m_ptr = element;
		}
		void insert(size_t index, T element)
		{
			if (!is_valid || index >= max_size)return;

			if (next_pos < max_size)
			{
				T* temp_ptr = reinterpret_cast<T*>(calloc(max_size, sizeof(T)));

				for (size_t i = 0; i < max_size; ++i)
				{
					if (i < index)temp_ptr[i] = m_ptr[i];
					else if (i == index)temp_ptr[i] = element;
					else temp_ptr[i + 1] = m_ptr[i];
				}

				free(m_ptr);
				m_ptr = temp_ptr;
				++next_pos;
				return;
			}
			else
			{
				T* temp_ptr = reinterpret_cast<T*>(calloc(max_size + 1, sizeof(T)));

				for (size_t i = 0; i < max_size; ++i)
				{
					if (i < index)temp_ptr[i] = m_ptr[i];
					else if (i == index)temp_ptr[i] = element;
					else temp_ptr[i + 1] = m_ptr[i];
				}

				free(m_ptr);
				m_ptr = temp_ptr;
				++next_pos;
				return;
			}

		}

		T& front()
		{
			return m_ptr[0];
		}
		T& back()
		{
			return m_ptr[next_pos - 1];
		}

		T& operator[](size_t index)
		{
			T dummy{};

			if (!is_valid || index >= next_pos)return dummy;

			return m_ptr[index];
		}
	};

	class SEADLL_API PROTON
	{
	protected:
		float width{ 1.0f };
		float height{ 1.0f };

		float x_radius{ 0 };
		float y_radius{ 0 };

	public:
		FPOINT start{ 0,0 };
		FPOINT end{ 0,0 };
		FPOINT center{ 0,0 };

		PROTON() {};
		PROTON(float _sx, float _sy, float _width, float _height);
		PROTON(FPOINT _start_pos, float _width, float _height);

		virtual ~PROTON() {};

		float GetWidth()const;
		float GetHeight()const;
		float XRadius()const;
		float YRadius()const;

		void SetEdges();
		void NewDims(float _new_width, float _new_height);

		void NewWidth(float _new_width);
		void NewHeight(float _new_height);
	};

	class SEADLL_API OBJECT :public PROTON
	{
	protected:
		types type = types::no_type;

		int current_frame{ 0 };
		int max_frames{ 0 };
		int frame_delay{ 0 };

		OBJECT(types _what, float _to_x, float _to_y);

	public:
		dirs dir = dirs::stop;
		
		virtual ~OBJECT() {};

		types GetType() const;
		int GetFrame();

		void SetType(types what);
		void ObjRelease();

		friend SEADLL_API OBJECT* CreateObject(types what, float to_x, float to_y);
	};

	class SEADLL_API CREATURES :public OBJECT
	{
	private:

		float move_sx{ 0 };
		float move_sy{ 0 };
		float move_ex{ 0 };
		float move_ey{ 0 };

		float slope{ 0 };
		float intercept{ 0 };

		float speed{ 0 };
		
		bool hor_dir = false;
		bool ver_dir = false;

		float sight_scope{ 0 };

		int patrol_move_points = 200;

		CREATURES(types __what, float __start_x, float __start_y, float __to_x, float __to_y);

		void SetPathInfo(float _to_where_x, float _to_where_y);

	public:
		states state = states::stop;

		friend SEADLL_API CREATURES* CreateCreature(types what, float start_x, float start_y, float to_x, float to_y);

		bool Move(float gear);
		bool Move(float gear, float dest_x, float dest_y);

		void EvilAI(PACK<FPOINT>& pack, float gear);

		void Release();
	};

	typedef OBJECT* GameObject;
	typedef CREATURES* GameCreature;

	// FUNCTIONS ************************************

	float SEADLL_API Distance(FPOINT my_point, FPOINT target_point);
	
	void Sort(PACK<FPOINT>& pack, FPOINT target)
	{
		bool sorted = false;

		while (!sorted)
		{
			sorted = true;

			for (size_t i = 0; i < pack.size() - 1; ++i)
			{
				if (Distance(pack[i], target) > Distance(pack[i + 1], target))
				{
					FPOINT temp = pack[i];
					pack[i] = pack[i + 1];
					pack[i + 1] = temp;
					sorted = false;
					break;
				}
			}
		}
	}

	SEADLL_API OBJECT* CreateObject(types what, float to_x, float to_y);

	SEADLL_API CREATURES* CreateCreature(types what, float start_x, float start_y, float to_x, float to_y);
}
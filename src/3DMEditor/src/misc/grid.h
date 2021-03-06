#ifndef __TA3D_GRID_H__
#define __TA3D_GRID_H__

template<class T>
class Grid
{
public:
	Grid();
	Grid(int w, int h);
	~Grid();
	void resize(int w, int h);

	const T& operator()(int x, int y) const;
	T& operator()(int x, int y);

	inline T *pointerToData()	{	return data;	}

	int getWidth() const {	return w;	}
	int getHeight() const {	return h;	}
	void add(const Grid<T> &grid, int x, int y);
	void sub(const Grid<T> &grid, int x, int y);
	void clear();

private:
	int w;
	int h;
	T *data;
};

void gaussianFilter(Grid<float> &grid, float sigma);

#include "grid.hxx"

#endif

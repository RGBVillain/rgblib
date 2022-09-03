#include "rgb_coord.h"

int rgb::rasterRectangle (const SRectangle & rectangle, const std::function<int(const ::rgb::SCoord &)> & funcSetPixel) {
	int					cellsDrawn		= 0;
	for(int y = rectangle.Position.y, yStop = rectangle.Position.y + rectangle.Size.y; y < yStop; ++y)
	for(int x = rectangle.Position.x, xStop = rectangle.Position.x + rectangle.Size.x; x < xStop; ++x)
		cellsDrawn		+= funcSetPixel(SCoord{x, y});

	return cellsDrawn;
}

int rgb::rasterCircle	(const SCircle & circle, const std::function<int(const ::rgb::SCoord &)> & funcSetPixel) {
	const SRectangle	enclosingRectangle	= 
		{ {int(circle.Center.x - circle.Radius), int(circle.Center.y - circle.Radius)}
		, {int(circle.Radius * 2), int(circle.Radius * 2)}
		};

	return rasterRectangle(enclosingRectangle, [circle, funcSetPixel] (const SCoord & cellCoord) {
		if((cellCoord - circle.Center).length() <= circle.Radius)
			return funcSetPixel(cellCoord);
		return 0;
	});
}

int rgb::rasterSegment	(::rgb::SCoord A, ::rgb::SCoord B, const std::function<int(const ::rgb::SCoord &)> & funcSetPixel) {
	int				countPixels				= 0;
	::rgb::SCoord	difference				= {(B.x - A.x), (B.y - A.y)};
	bool			useX					= abs(difference.x) > abs(difference.y);

	if(useX) {
		if(A.x > B.x) {
			std::swap(A, B);
			difference		= B - A;
		}
		float			slope					= difference.y / float(difference.x);
		float			y						= float(A.y) + .5f;
		for(float x = A.x + .5f, xStop = B.x + .5f; x < xStop; ++x) {
			countPixels	+= funcSetPixel({int(x), int(y)});
			y			+= slope;
		}
	}
	else {
		if(A.y > B.y) {
			std::swap(A, B);
			difference		= B - A;
		}
		float			slope					= difference.x / float(difference.y);
		float			x						= float(A.x) + .5f;
		for(float y = A.y + .5f, yStop = B.y + .5f; y < yStop; ++y) {
			countPixels	+= funcSetPixel({int(x), int(y)});
			x			+= slope;
		}
	}
	return countPixels;
}

#if !defined(__GUMPIMAGE_H__)
#define __GUMPIMAGE_H__

#include "gui/image.h"

class cGumpImage : public cControl {
protected:
	cTexture *texture;
	unsigned short id_;
	unsigned short hue_;
	bool partialHue_;
	bool autoSize_;
public:
	void update();

	cGumpImage(unsigned short id, unsigned short hue = 0, bool partialHue = false, bool autoSize = true);
	virtual ~cGumpImage();

	inline bool autoSize() const { return autoSize_; }
	inline unsigned short id() const { return id_; }
	inline unsigned short hue() const { return hue_; }
	inline bool partialHue() const { return partialHue_; }

	void draw(int xoffset, int yoffset);

	inline void setAutoSize(bool data) {
		autoSize_ = data;
		if (texture) {
			int width = texture->realWidth();
			int height = texture->realHeight();
			if (width_ != width || height_ != height) {
				setSize(width, height);
			}
		}
	}

	inline void setId(unsigned short id) {
		if (id_ != id) {
			id_ = id;
			if (texture) {
				texture->decref();
				texture = 0;
			}
			invalidate();
		}
	}

	inline void setHue(unsigned short hue) {
		if (hue != hue_) {
			hue_ = hue;
			if (texture) {
				texture->decref();
				texture = 0;
			}
			invalidate();
		}
	}

	inline void setPartialHue(bool partialhue) {
		if (partialhue != partialHue_) {
			partialHue_= partialhue;
			if (texture) {
				texture->decref();
				texture = 0;
			}
			invalidate();
		}
	}
};

#endif

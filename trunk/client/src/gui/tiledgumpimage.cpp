
#include "exceptions.h"
#include "engine.h"
#include "utilities.h"
#include "muls/gumpart.h"
#include "gui/tiledgumpimage.h"

void cTiledGumpImage::update() {
	if (!texture) {
		texture = Gumpart->readTexture(id_, hue_, partialHue_);
		texture->incref();
	}
	dirty_ = false;
}

cTiledGumpImage::cTiledGumpImage(unsigned short id, unsigned short hue, bool partialHue) {	
	id_ = id;
	hue_ = hue;
	partialHue_ = partialHue;
	texture = 0;
	moveHandle_ = true;
}

cTiledGumpImage::~cTiledGumpImage() {
	if (texture) {
		texture->decref();
	}
}

void cTiledGumpImage::draw(int xoffset, int yoffset) {
	if (isDirty()) {
		update();
	}

	// Create a QUAD STRIP
	if (texture) {
		texture->bind();

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // White. No Alpha.

		int xcount = (width_ + texture->realWidth() - 1) / texture->realWidth();
		int ycount = (height_  + texture->realHeight() - 1) / texture->realHeight();

		int drawy = yoffset + y_;
		int bottom = drawy + texture->height(); // Visible area bottom.
		float texturex = 1.0; // High texture Coordinate
		float texturey = 1.0; // High texture coordinate
		int maxright = xoffset + x_ + width_;
		int maxbottom = drawy + height_;
		int tHeight = texture->height(); // Real height of the texture
		int tWidth = texture->width(); // Real width of the texture

		for (int y = 0; y < ycount; ++y) {
			if (bottom > maxbottom) {
                bottom = maxbottom;
				texturex = 1.0;
				texturey = (bottom - drawy) / (float)tHeight;
			} else {
				texturex = 1.0;
				texturey = 1.0;
			}

			int drawx = xoffset + x_; // Where to draw the texture
			int right = drawx + texture->width(); // Coordinate of the last visible right pixel.

			glBegin(GL_QUAD_STRIP); // Begin a quad strip for this row of the tiled image.

			// Starting the tiling process for this row
			for (int x = 0; x < xcount; ++x) {
				// This clips this row into the clipping rectangle of this control
				if (right > maxright) {
                    right = maxright;
					// Modify the texel coordinates
					texturex = (right - drawx) / (float)tWidth;
				}

				glTexCoord2f(0, 0); glVertex2f(drawx, drawy); // Upper left corner
				glTexCoord2f(0, texturey); glVertex2f(drawx, bottom); // Lower Left Corner
				glTexCoord2f(texturex, 0); glVertex2f(right, drawy); // Upper Right Corner
				glTexCoord2f(texturex, texturey); glVertex2f(right, bottom); // Lower Right Corner

				// Advance to the next element in the row
				drawx += texture->realWidth();
				right += texture->realWidth();
			}

			glEnd();

			// Advance to the next row
			drawy += texture->realHeight();
			bottom += texture->realHeight();
		}		
	}
}

/*
  Copyright (C) 2007,2008 Richard Quirk

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "libnds.h"
#include "Canvas.h"
#include "Document.h"
#include "HeaderParser.h"
#include "HtmlDocument.h"
#include "ImageComponent.h"
#include "Image.h"
#include "Palette.h"
#include "URI.h"

using nds::Canvas;
using nds::Image;


ImageComponent::ImageComponent(nds::Image * image, Document * doc):m_image(image), m_document(doc)
{
  if (m_image)
  {
    setSize(m_image->width(), m_image->height());
  }
  if (m_document)
  {
    m_document->registerView(this);
  }
}

ImageComponent::~ImageComponent()
{
  delete m_image;
}

static void drawImage(Canvas & canvas, nds::Image & image, int startx, int starty)
{
  const unsigned short * data = image.data();
  for (unsigned int y = 0; y < image.height(); ++y)
  {
    const unsigned short * row = &data[y*image.width()];
    for (unsigned int x = 0; x < image.width(); ++x, row++)
    {
      canvas.drawPixel(startx+x, starty+y, *row);
    }
  }
}

void ImageComponent::paint(const nds::Rectangle & clip)
{
  if (m_image)
  {
    //nds::Canvas::instance().drawRectangle(x(),y(),width(),height(), nds::Color(31,0,0));
    drawImage(Canvas::instance(), *m_image, x(), y());
  }
  else
  {
    Canvas::instance().drawRectangle(0,0,128,128, nds::Color(31,0,0));
  }
  m_dirty = false;
}

void ImageComponent::reload()
{
  if (m_image->type() == nds::Image::ImageUNKNOWN)
  {
    m_image->setType((nds::Image::ImageType)m_document->htmlDocument()->mimeType());
  }
  m_image->reload();
  setSize(m_image->width(), m_image->height());
  m_dirty = true;
}
void ImageComponent::notify()
{
  if ( m_document->headerParser().cacheFile() == (m_image->filename()+".hdr"))
  {
    if (m_document->status() == Document::INPROGRESS)
    {
      reload();
    }
    if (m_document->status() == Document::LOADED)
    {
      reload();
      m_document->unregisterView(this);
    }
  }
}

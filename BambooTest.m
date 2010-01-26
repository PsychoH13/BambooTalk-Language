#:
 Copyright (c) 2010, Remy Demarest
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY Remy Demarest ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
:#

namespace AppKit
{
    import Foundation.
    
    struct Point
    {
        - Float x, y.
        
        @ (self)newWithX:(Float)x y:(Float)y
        {
            self x := x.
            self y := y.
        }
        
        * (self)newWithString:(String)aString
        {
            Array[Float] $ ret := aString scanWithFormat: "{ %g, %g }".
            
            self newWithX: ret[0] y: ret[1].
        }
        
        + (self)zero { return self new. }
        
        - (Point) + (Point)p    { return self copy setX: (x + p x); setY: (y + p y); self. }
        - (Boolean) == (Point)p { return (x == p x) && (y == p y). }
        - (Boolean)isZero       { return self == Zero. }
        - (String)description   { return String stringWithFormat:"{ %g, %g }" arguments: #(x, y). }
        - (Point)copy           { return Point new setX: x; setY: y; self. }
    }
    
    struct Size
    {
        - Float width, height.
        
        @ (self)newWithWidth:(Float)w height:(Float)h
        {
            width  := w.
            height := h.
        }
        
        * (self)newWithString:(String)aString
        {
            Array[Float] $ ret := aString scanWithFormat: "{ %g, %g }".
            
            self newWithWidth: ret[0] height: ret[1].
        }
        
        + (self)zero { return self new. }
        
        - (Boolean) == (Size)s { return (width == s width) && (height == s height). }
        - (Boolean)isZero      { Size $ s := new. return self == s. }
        - (String)description  { return String stringWithFormat:"{ %g, %g }" arguments: #(width, height). }
        - (Size)copy           { return Size new setWidth: width; setHeight: height; self. }
    }
    
    struct Rect
    {
        enum Edge
        {
            MinX := 0,
            MinY := 1,
            MaxX := 2,
            MaxY := 3
        }
        
        - Point origin.
        - Size  size.
        
        @ (self)newWithOrigin:(Point)anOrigin size:(Size)aSize
        {
            origin := anOrigin.
            size   := aSize.
        }
        
        * (self)new.
        * (self)newWithX:(Float)x y:(Float)y width:(Float)w height:(Float)h
        {
            self newWithOrigin: (Point newWithX:x y:y) size: (Size newWithWidth:w height:h). 
        }
        
        * (self)newWithString:(String)aString
        {
            Array[Float] $ ret := aString scanWithFormat: "{ { %g, %g }, { %g, %g } }".
            
            self newWithX: ret[0] y: ret[1] width: ret[2] height: ret[3].
        }
        
        + (self)zero { return self new. }
        
        - (Float)minX          { return origin x. }
        - (Float)minY          { return origin y. }
        - (Float)midX          { return origin x + (size width  / 2). }
        - (Float)midY          { return origin y + (size height / 2). }
        - (Float)maxX          { return origin x + size width.  }
        - (Float)maxY          { return origin y + size height. }
        - (Float)width         { return size width.  }
        - (Float)height        { return size height. }
        - (Boolean)isZero      { return origin isZero && size isZero. }
        - (Boolean)isEmpty     { return ((size width > 0.0) && (size height > 0.0)) not. }
        - (Boolean) == (Rect)r { return (origin == r origin) && (size == r size). }
        - (String)description  { return String stringWithFormat:"{ %@, %@ }" arguments: #(origin, size). }
        - (Rect)copy           { return Rect new setOrigin: (origin copy); setSize: (size copy); self. }
        
        - (Rect)insetByX:(Float)x y:(Float)y
        {
            return self copy setOrigin:(origin copy setX: (origin x + x);
                                                    setY: (origin y + y);
                                                    self);
                             setSize:  (size copy setWidth:  (width  - (x * 2));
                                                  setHeight: (height - (x * 2)); 
                                                  self);
                             self.
        }
        
        - (Rect) | (Rect)r
        {
            if(self isEmpty)
                if(r isEmpty)
                    return Rect new.
                else
                    return r.
            else if(r isEmpty)
                return self.
                            
            Point $ o := Point new setX: (self minX ?< r minX);
                                   setY: (self minY ?< r minY);
                                   self.
            Size $ s := Size new setWidth:  (self maxX ?> r maxX - o x);
                                 setHeight: (self maxY ?> r maxY - o y);
                                 self.
            
            return Rect new setOrigin: o; setSize: s; self.
        }
        - (Rect)unionWith:(Rect)r { return self | r. }
        
        - (Rect) & (Rect)r
        {
            if((self maxX <= r minX) ||
               (self minX >= r maxX) ||
               (self maxY <= r minY) ||
               (self minY >= r maxY))
                return Rect new.
                
            Point $ o := Point new setX: (self minX ?> r minX);
                                   setY: (self minY ?> r minY);
                                   self.
            Size $ s := Size new setWidth:  (self maxX ?< r maxX - o x);
                                 setHeight: (self maxY ?< r maxY - o y);
                                 self.
            return Rect new setOrigin: o; setSize: s; self.
        }
        - (Rect)intersectionWith:(Rect)r { return self & r. }
        
        - ({Rect slice, remainder})divideByAmount:(Float)amount fromEdge:(Edge)edge
        {
            {Rect slice, remainder} $ ret := { self copy, self copy }.
            
            Float temp.
            
            switch(edge)
            {
                case Edge MinX :
                    ret slice     size   width := amount.
                    ret remainder size   width := - amount.
                    ret remainder origin x     := + amount.
                    break.
                case Edge MinY :
                    ret slice     size   height := amount.
                    ret remainder size   height := - amount.
                    ret remainder origin y      := + amount.
                    break.
                case Edge MaxX :
                    temp := ret slice width - amount.
                    ret remainder size   width := temp.
                    ret slice     size   width := amount.
                    ret slice     origin x     := + temp.
                    break.
                case Edge MaxY :
                    temp := ret slice height - amount.
                    ret remainder size   height := temp.
                    ret slice     size   height := amount.
                    ret slice     origin y      := + temp.
                    break.
            }
            
            return ret.
        }
        
        - (Rect)offsetByX:(Float)x y:(Float)y
        {
            return self copy setX: (origin x + x);
                             setY: (origin y + y);
                             self.
        }
        
        - (Rect)integralRect
        {
            Rect $ ret := self copy.
            if(ret isEmpty not)
            {
                ret origin setX: (origin x floor);
                           setY: origin y floor.
                ret size setWidth:  (size width ceiling);
                         setHeight: size height ceiling.
            }
            
            return ret.
        }
        
        - (Boolean)containsRect:(Rect)r
        {
            return ((self minX < r minX) && (self minY < r minY) &&
                    (self maxX > r maxX) && (self maxY > r maxY)). 
        }
        
        - (Boolean)intersectsWithRect:(Rect)r
        {
            return ((self maxX <= r minX) || (self minX >= r maxX) ||
                    (self maxY <= r minY) || (self minY >= r maxY)) not.
        }
        
        - (Boolean)containsPoint:(Point)p
        {
            return ((p x >= self minX) && (p x <= self maxX) &&
                    (p y >= self minY) && (p y <= self minY)).
        }
    }
    
    struct BezierPath <Copying, Coding>
    {
        enum LineCapStyle
        {
            ButtLine := 0,
            RoundLine := 1,
            SquareLine := 2
        }
        
        enum LineJoinStyle
        {
            Miter := 0,
            Round := 1,
            Bevel := 2
        }
        
        enum WindingRule
        {
            NonZero := 0,
            EvenOdd := 1
        }
        
        enum ElementType
        {
            MoveTo,
            LineTo,
            CurveTo,
            ClosePath
        }
        
        + Float defaultMiterLimit.
        + Float defaultFlatness.
        + WindingRule defaultWindingRule.
        + LineCapStyle defaultLineCapStyle.
        + LineJoinStyle defaultLineJoinStyle.
        + Float defaultLineWidth.
        
        - Float lineWidth.
        - LineCapStyle lineCapStyle.
        - LineJoinStyle lineJoinStyle.
        - WindingRule windingRule.
        - Float miterLimit.
        - Float flatness.
        
        - (Point)currentPoint
        {
            Point ret := Point new.
            if(elements count > 0) ret := elements lastObject point copy.
            return ret.
        }
        
        - (void)moveToPoint:(Point)p
        {
            elements addObject: (Element new setType:  (ElementType MoveTo);
                                             setPoint: p;
                                             self).
        }
        - (void)lineToPoint:(Point)p
        {
            elements addObject: (Element new setType:  (ElementType LineTo);
                                             setPoint: p;
                                             self).
        }
        - (void)curveToPoint:(Point)point controlPoint1:(Point)cp1 controlPoint2:(Point)cp2
        {
            elements addObject: (Element new setType:          (ElementType CurveTo);
                                             setPoint:         point;
                                             setControlPoints: #(cp1, cp2);
                                             self).
        }
        - (void)closePath
        {
            elements addObject: (Element new setType: (ElementType CurveTo); self).
        }
        
        - (void)relativeMoveToPoint:(Point)point
        {
            self moveToPoint: self currentPoint + point.
        }
        
        - (void)relativeLineToPoint:(Point)point
        {
            self lineToPoint: self currentPoint + point.
        }
        
        - (void)relativeCurveToPoint:(Point)point controlPoint1:(Point)cp1 controlPoint2:(Point)cp2
        {
            Point p := self currentPoint.
            
            self relativeCurveToPoint: p + point controlPoint1: p + cp1 controlPoint2: p + cp2.
        }
        
        - (void)appendBezierPathWithPoints:(Array[Point])points
        {
            
            for(Point p at Integer idx in points)
            {
                if(idx == 0) self moveToPoint:p.
                else self lineToPoint:p.
            }
        }
        
        - (void)appendBezierPathWithRect:(Rect)rect
        {
            self moveToPoint: (Point new setX: (rect minX); setY: (rect minY); self).
            self lineToPoint: (Point new setX: (rect maxX); setY: (rect minY); self).
            self lineToPoint: (Point new setX: (rect maxX); setY: (rect maxY); self).
            self lineToPoint: (Point new setX: (rect minX); setY: (rect maxY); self).
            self closePath.
        }
        
        - Integer elementCount { get { return elements count. } }
        
        private
        typedef Element {
            ElementType  type.
            Point        point.
            Array[Point] controlPoints.
        }.
        
        - Array[Element] elements.
    }
}

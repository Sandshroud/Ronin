/**
 @file Color1uint8.cpp
 
 @author Morgan McGuire, http://graphics.cs.williams.edu
  
 @created 2007-01-30
 @edited  2007-01-30
 */

#include "platform.h"
#include "g3dmath.h"
#include "Color1uint8.h"
#include "Color1.h"
#include "BinaryInput.h"
#include "BinaryOutput.h"

namespace G3D {

Color1uint8::Color1uint8(const class Color1& c) : value(iClamp(iFloor(c.value * 256), 0, 255)) {
}


Color1uint8::Color1uint8(class BinaryInput& bi) {
    deserialize(bi);
}


void Color1uint8::serialize(class BinaryOutput& bo) const {
    bo.writeUInt8(value);
}


void Color1uint8::deserialize(class BinaryInput& bi) {
    value = bi.readUInt8();
}


}

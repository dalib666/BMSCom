#pragma once

inline bool isTempValid(float val){
    return ((val < 40.0f) && (val > -20.0f));
}
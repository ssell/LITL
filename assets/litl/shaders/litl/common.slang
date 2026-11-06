// -----------------------------------------------------------------------------------------
// [common.slang]
// Core shader functionality and utility for both internal and external LITL shaders.
// -----------------------------------------------------------------------------------------

module common;

public static const uint BindSetPerFrame = 0;
public static const uint BindSetPerPass = 1;
public static const uint BindSetPerMaterial = 2;
public static const uint BindSetPerObject = 3;

public static const float PI          = 3.1415926f;
public static const float PI_OVER_TWO = 1.5707963f;
public static const float ONE_OVER_PI = 0.3183098f;
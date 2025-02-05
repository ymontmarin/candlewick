
float linearizeDepth(float depth, float zNear, float zFar) {
    float z_ndc = 2.0 * depth - 1.0;
    return (2.0 * zNear * zFar) / (zFar + zNear - z_ndc * (zFar - zNear));
}

float linearizeDepthOrtho(float depth, float zNear, float zFar) {
    return zNear + depth * (zFar - zNear);
}

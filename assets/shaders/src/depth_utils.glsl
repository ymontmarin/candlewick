
float linearizeDepth(float depth, float near, float far) {
    float z_ndc = 2.0 * depth - 1.0;
    return (2.0 * near * far) / (far + near - z_ndc * (far - near));
}

float linearizeDepthOrtho(float depth, float near, float far) {
    return near + depth * (far - near);
}


#define PI 3.14159265358979323846

float refractionAngle(float n1, float theta1, float n2) {
    float sin2 = n1 * sin(theta1) / n2;
    float theta2 = asin(sin2);
    return theta2;
}

vec2 getPhase(float lambda, float opd) {
    float m = (opd/lambda) * 2.0 * PI;
    return vec2(cos(m), sin(m));
}

float calcInterference(float lambda, float opd) {
    float ur = 0.5;     // Upper Reflection
    float lr = 1.0;     // Lower Reflection
    float si = 1.0;     // Start Intensity
    float ri = si - ur; // Refracted Intensity
    vec2  reflected = si * ur * vec2(1.0, 0.0);
    vec2  refracted = ri * lr * getPhase(lambda, opd);
    vec2  interference = reflected - refracted;
    float sensitivity  = dot(interference, interference);
    return sensitivity;
}

float getOPD(float d, float theta, float n) {
    return n * 2.0 * d * cos(theta);
}

float power(vec2 l) {
    return dot(l, l);
}

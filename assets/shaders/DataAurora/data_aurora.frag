#version 330 core
// Северное сияние, которое реагирует на данные (узел ShaderParams в редакторе).
//   u_temperature - градусы Цельсия: холодно -> синий/бирюзовый, жарко -> оранжевый/малиновый
//   u_energy      - 0..1: скорость и яркость полос
//   u_alert       - 0..1: красная пульсация поверх (например, от правила "слишком долго за компьютером")
// Все uniform-ы по умолчанию 0 - шейдер красиво выглядит и без данных.

out vec4 fragColor;
in vec2 fragCoord;

uniform vec2 iResolution;
uniform float iTime;

uniform float u_temperature;
uniform float u_energy;
uniform float u_alert;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash(i), hash(i + vec2(1.0, 0.0)), u.x),
               mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x), u.y);
}

float fbm(vec2 p) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 5; ++i) {
        value += amplitude * noise(p);
        p *= 2.03;
        amplitude *= 0.5;
    }
    return value;
}

void main() {
    vec2 uv = fragCoord / iResolution;
    float aspect = iResolution.x / max(iResolution.y, 1.0);
    vec2 p = vec2(uv.x * aspect, uv.y);

    float energy = clamp(u_energy, 0.0, 1.0);
    float warm = clamp((u_temperature + 15.0) / 45.0, 0.0, 1.0);  // -15C -> 0, +30C -> 1
    float t = iTime * (0.08 + energy * 0.35);

    // Ночное небо: тёмный вертикальный градиент, чуть теплее при жаре
    vec3 sky_top = mix(vec3(0.01, 0.02, 0.06), vec3(0.06, 0.02, 0.05), warm);
    vec3 sky_bottom = mix(vec3(0.02, 0.06, 0.12), vec3(0.14, 0.05, 0.06), warm);
    vec3 color = mix(sky_bottom, sky_top, uv.y);

    // Звёзды
    vec2 star_cell = floor(p * 90.0);
    float star = step(0.9965, hash(star_cell)) * (0.6 + 0.4 * sin(iTime * 3.0 + hash(star_cell) * 40.0));
    color += vec3(star) * (1.0 - uv.y * 0.3);

    // Цвета сияния: от холодной палитры к тёплой
    vec3 cold_a = vec3(0.10, 0.95, 0.65);
    vec3 cold_b = vec3(0.20, 0.45, 1.00);
    vec3 hot_a  = vec3(1.00, 0.55, 0.15);
    vec3 hot_b  = vec3(0.95, 0.15, 0.45);
    vec3 band_a = mix(cold_a, hot_a, warm);
    vec3 band_b = mix(cold_b, hot_b, warm);

    // Несколько волнистых полос, искажённых шумом
    float intensity = 0.9 + energy * 0.9;  // красиво и без данных, ярче с активностью
    for (int i = 0; i < 4; ++i) {
        float fi = float(i);
        float wave = fbm(vec2(p.x * 1.3 + t * (1.0 + fi * 0.35) + fi * 7.0, t * 0.5 + fi));
        float center = 0.45 + fi * 0.08 + (wave - 0.5) * 0.45;
        float dist = uv.y - center;
        // Резкий нижний край и мягкий "занавес" вверх, как у настоящего сияния
        float curtain = exp(-max(dist, 0.0) * (9.0 - fi)) * smoothstep(-0.02, 0.0, dist);
        float rays = 0.6 + 0.4 * fbm(vec2(p.x * 18.0 + fi * 3.0, t * 2.0));
        vec3 band = mix(band_a, band_b, clamp(dist * 4.0 + 0.3 + fi * 0.15, 0.0, 1.0));
        color += band * curtain * rays * intensity * (0.35 - fi * 0.05);
    }

    // Тревога: красная пульсация
    float alert = clamp(u_alert, 0.0, 1.0) * (0.5 + 0.5 * sin(iTime * 5.0));
    color = mix(color, vec3(0.9, 0.05, 0.05), alert * 0.35);

    // Лёгкая виньетка
    color *= 1.0 - 0.35 * length(uv - 0.5);
    fragColor = vec4(color, 1.0);
}

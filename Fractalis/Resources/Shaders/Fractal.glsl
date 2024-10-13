#version 450


struct Complex
{
	double Re;
	double Im;
};

struct UBO
{
	int Fractal;
	int Color;
	Complex C;
	int MaxIteration;
	float Zoom;
	dvec2 Offset;
};

layout (binding = 0, rgba8) uniform image2D u_Image;
layout (binding = 1, std140) readonly buffer SSBO { UBO u_UBO; };
layout (local_size_x = 16, local_size_y = 16) in;


Complex AddComplex(Complex z1, Complex z2)
{
	return Complex(z1.Re + z2.Re, z1.Im + z2.Im);
}

Complex MultiplyComplex(Complex z1, Complex z2)
{
	return Complex(z1.Re * z2.Re - z1.Im * z2.Im, z1.Re * z2.Im + z1.Im * z2.Re);
}

double Modulus(Complex z)
{
	return sqrt(z.Re * z.Re + z.Im * z.Im);
}




vec3 Temperature(float t)
{
	vec3 blue = vec3(0.0, 0.0, 1.0);
    vec3 cyan = vec3(0.0, 1.0, 1.0);
    vec3 yellow = vec3(1.0, 1.0, 0.0);
    vec3 red = vec3(1.0, 0.0, 0.0);

    if (t < 0.33) return mix(blue, cyan, t * 3.0);
	else if (t < 0.66) return mix(cyan, yellow, (t - 0.33) * 3.0);
    else return mix(yellow, red, (t - 0.66) * 3.0);
}

vec3 Cosmic(float t)
{
	vec3 black = vec3(0.0, 0.0, 0.0);
	vec3 darkBlue = vec3(0.1, 0.15, 0.23);
	vec3 blue = vec3(0.25, 0.35, 0.47);
	vec3 green = vec3(0.61, 0.77, 0.24);
	vec3 orange = vec3(0.95, 0.39, 0.01);
	vec3 magenta = vec3(0.85, 0.01, 0.41);
	vec3 white = vec3(0.92, 0.92, 0.92);

	if (t < 0.17) return mix(black, darkBlue, t * 6.0);
	else if (t < 0.33) return mix(darkBlue, blue, (t - 0.17) * 6.0);
	else if (t < 0.50) return mix(blue, green, (t - 0.33) * 6.0);
	else if (t < 0.67) return mix(green, orange, (t - 0.50) * 6.0);
	else if (t < 0.84) return mix(orange, magenta, (t - 0.67) * 6.0);
	else return mix(magenta, white, (t - 0.84) * 6.0);
}

vec3 Ocean(float t)
{
	vec3 darkBlue = vec3(0.0, 0.0, 0.2);
    vec3 blue = vec3(0.0, 0.0, 1.0);
    vec3 cyan = vec3(0.0, 1.0, 1.0);
    vec3 white = vec3(1.0, 1.0, 1.0);

    if (t < 0.5) return mix(darkBlue, blue, t * 2.0);
    else return mix(blue, white, (t - 0.5) * 2.0);
}

vec3 Fire(float t)
{
	vec3 black = vec3(0.0, 0.0, 0.0);
    vec3 red = vec3(1.0, 0.0, 0.0);
    vec3 orange = vec3(1.0, 0.5, 0.0);
    vec3 yellow = vec3(1.0, 1.0, 0.0);
    vec3 white = vec3(1.0, 1.0, 1.0);

    if (t < 0.25) return mix(black, red, t * 4.0);
    else if (t < 0.5) return mix(red, orange, (t - 0.25) * 4.0);
    else if (t < 0.75) return mix(orange, yellow, (t - 0.5) * 4.0);
    else return mix(yellow, white, (t - 0.75) * 4.0);
}

vec3 BlackAndWhite(float t)
{
	return mix(vec3(0.0), vec3(1.0), t);
}

vec4 GetColor(float t)
{
	switch (u_UBO.Color)
	{
		case 0:		return vec4(Cosmic(t), 1.0);
		case 1:		return vec4(Temperature(t), 1.0);
		case 2:		return vec4(Ocean(t), 1.0);
		case 3:		return vec4(Fire(t), 1.0);
		case 4:		return vec4(BlackAndWhite(t), 1.0);
	}
}

vec4 Julia(Complex z)
{
	int i = 0;

	while (Modulus(z) <= 100.0 && i < u_UBO.MaxIteration)
	{
		z = AddComplex(MultiplyComplex(z, z), u_UBO.C);
		i++;
	}

	if (i == u_UBO.MaxIteration)
		return vec4(0.0);
	else
	{
		float value = float(i) / float(u_UBO.MaxIteration);
		return GetColor(value);
	}
}

vec4 Mandelbrot(Complex c)
{
	int i = 0;
	Complex z = Complex(0, 0);

	while (Modulus(z) <= 100.0 && i < u_UBO.MaxIteration)
	{
		z = AddComplex(MultiplyComplex(z, z), c);
		i++;
	}

	if (i == u_UBO.MaxIteration)
		return vec4(0.0);
	else
	{
		float value = float(i) / float(u_UBO.MaxIteration);
		return GetColor(value);
	}
}



void main()
{
	dvec2 imgSize = dvec2(imageSize(u_Image).xy);
	dvec2 coords = dvec2(gl_GlobalInvocationID.xy) / imgSize * 2.0 - 1.0;
	coords.x *= imgSize.x / imgSize.y;
	coords *= u_UBO.Zoom;
	coords += u_UBO.Offset;

	Complex z = Complex(coords.x, coords.y);

	vec4 finalColor = vec4(0.0);
	switch (u_UBO.Fractal)
	{
		case 0:
			finalColor = Julia(z);
			break;
		case 1:
			finalColor = Mandelbrot(z);
			break;
	}

	imageStore(u_Image, ivec2(gl_GlobalInvocationID.xy), finalColor);
}

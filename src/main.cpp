#include "ofMain.h"
#include "ofApp.h"

#define RUN_TEST 0

#include "online.hpp"
#include "peseudo_random.hpp"
#include "coordinate.hpp"
#include "randomsampler.hpp"
// #include "SphericalRectangleSamplerCoordinate.hpp"
#include "SphericalRectangleSamplerCoordinate_Optimized.hpp"

/*
 任意の平面で、Area Uniformなサンプリングと、SolidAngle Uniformなサンプリングによる積分は、一致しなければならない。
*/
inline void simple_shading_check(int seed) {
	rt::XoroshiroPlus128 random(seed);
	glm::dvec3 o = glm::dvec3(random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0), random.uniform(-1.0, 1.0));
	glm::dvec3 ex = rt::sample_on_unit_sphere(&random);

	rt::Rectangle light = rt::Rectangle(
		glm::dvec3(random.uniform(-1.0, 1.0), random.uniform(3.0, 4.0), random.uniform(-1.0, 1.0)),
		ex * random.uniform(1.0, 2.0),
		rt::ArbitraryBRDFSpace(ex).xaxis * random.uniform(1.0, 2.0)
	);

	rt::SphericalRectangleSamplerCoordinate sampler(light, o);

	double sr = sampler.solidAngle();

	rt::OnlineMean<double> Lo1;
	rt::OnlineMean<double> Lo2;
	glm::dvec3 Ng(0, 1, 0);
	glm::dvec3 LN = light.normal();


	printf("-- (seed: %d) --\n", seed);
	printf("sr: %.4f\n", sr);
	printf("area: %.4f\n", light.area());

	for (int i = 0; i < 1000000; ++i) {
		double Li = 10.0;
		double brdf = 1.0 / glm::pi<double>();
		{
			glm::dvec3 sample = light.sample(random.uniform(), random.uniform()); /* area uniform */
			glm::dvec3 wi = glm::normalize(sample - o);

			double G = glm::dot(Ng, wi) * glm::abs(glm::dot(LN, -wi)) / glm::distance2(sample, o);
			double pdf_area = 1.0 / light.area();
			double value = Li * brdf * G / pdf_area;
			Lo1.addSample(value);
		}

		{
			glm::dvec3 sample = sampler.sample(random.uniform(), random.uniform()); /* solidangle uniform */
			glm::dvec3 wi = glm::normalize(sample - o);
			double pdf_sr = 1.0 / sampler.solidAngle();
			double value = Li * brdf * glm::dot(Ng, wi) / pdf_sr;
			Lo2.addSample(value);
		}
		// for graph check
		// https://docs.google.com/spreadsheets/d/12isLUDUFe5WlRD9Zn6ByIJhgo4az62JZc6hNNvPx9b0/edit?usp=sharing
		//if (i % 5000 == 0) {
		//	printf("\"%.10f\",\"%.10f\",\"%.10f\"\n", Lo1.mean(), Lo2.mean(), Lo1.mean() - Lo2.mean());
		//}
	}
	printf("%.5f, %.5f, d = %.5f\n", Lo1.mean(), Lo2.mean(), Lo1.mean() - Lo2.mean());

	if (0.0005 < std::abs(Lo1.mean() - Lo2.mean())) {
		// any bug detected
		abort();
	}
}

//========================================================================
int main( ){
#if RUN_TEST
	for (int i = 0; i < 100; ++i) {
		simple_shading_check(i);
	}
	printf("done.\n");
	std::cin.get();
#else
	ofSetupOpenGL(1024,768,OF_WINDOW);
	ofRunApp(new ofApp());
#endif
}

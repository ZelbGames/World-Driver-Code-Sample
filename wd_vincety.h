#ifndef WDVINCETYA_H
#define WDVINCETY_H

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {
	class VincetyFunctions : public Resource {
		GDCLASS(VincetyFunctions, Resource)

	private:
		const double req = 6378137.0;             //Radius at equator
		const double flat =  1 / 298.257223563;    //flattening of earth
		const double rpol = (1 - flat) * req; //radius to the poles
		const double PI = 3.141592653589793;

		//sort into used by both and use by each
		double sin_sigma, cos_sigma, sigma, sin_alpha, cos_sq_alpha, cos2sigma;
		double C, lam_pre;

		double sigma1, two_sigma_m, sigma_pre;

	protected:
		static void _bind_methods();

	public:
		VincetyFunctions();
		~VincetyFunctions();

	public:
		Array calculate_distance(double longp, double latp, double longc, double latc);
		Array calculate_coordinates(double latp, double longp, double distance, double azimuth_deg);

	};
}

#endif
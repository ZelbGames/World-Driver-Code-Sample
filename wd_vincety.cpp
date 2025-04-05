#include "wd_vincety.h"
#include <godot_cpp/core/class_db.hpp>


//function below taken directly from TC Sample code, function is called to write data to a string
//check if functions outside of the name space work as well (they should)
static size_t my_write(void* buffer, size_t size, size_t nmemb, void* param)
{
	godot::String& text = *static_cast<godot::String*>(param);
	size_t totalsize = size * nmemb;
	text += (static_cast<char*>(buffer), totalsize);
	return totalsize;
}


using namespace godot;

void VincetyFunctions::_bind_methods() {
	ClassDB::bind_method(D_METHOD("calculate_distance"), &VincetyFunctions::calculate_distance);
	ClassDB::bind_method(D_METHOD("calculate_coordinates"), &VincetyFunctions::calculate_coordinates);

}

VincetyFunctions::VincetyFunctions() {
	// Initialize any variables here.
	//UtilityFunctions::print("Constructed");

}

VincetyFunctions::~VincetyFunctions() {
	//place the remove variables and data stuff here
	//memdelete(test_file);
	// Add your cleanup here.
}


Array VincetyFunctions::calculate_distance(double latp, double longp, double latc, double longc) {
	//UtilityFunctions::print("Calculating Distance");

	//for now use a PI value

	// convert to radians
	latp = PI * latp / 180.0; //PI is somewhere in godot but can't find it
	latc = PI * latc / 180.0;
	longp = PI * longp / 180.0;
	longc = PI * longc / 180.0;

	const double u1 = atan((1. - flat) * tan(latp));//godot::atan from the utilities function header file
	const double u2 = atan((1. - flat) * tan(latc));

	double lon = longc - longp; //difference in longtitude
	double lam = lon;
	const double tol = pow(10., -12);//0.01; //to the nearest cm //this is 10^-12?! pow(10., -12); // iteration tolerance
	double diff = 1.;

	while (abs(diff) > tol) {
		sin_sigma = sqrt(pow((cos(u2) * sin(lam)), 2.) + pow(cos(u1) * sin(u2) - sin(u1) * cos(u2) * cos(lam), 2.));
		cos_sigma = sin(u1) * sin(u2) + cos(u1) * cos(u2) * cos(lam);
		sigma = atan(sin_sigma / cos_sigma);
		sin_alpha = (cos(u1) * cos(u2) * sin(lam)) / sin_sigma;
		cos_sq_alpha = 1. - pow(sin_alpha, 2.);
		cos2sigma = cos_sigma - ((2. * sin(u1) * sin(u2)) / cos_sq_alpha);
		C = (flat / 16.) * cos_sq_alpha * (4. + flat * (4. - 3. * cos_sq_alpha));
		lam_pre = lam;
		lam = lon + (1. - C) * flat * sin_alpha * (sigma + C * sin_sigma * (cos2sigma + C * cos_sigma * (2. * pow(cos2sigma, 2.) - 1.)));
		diff = abs(lam_pre - lam);
	}

	const double usq = cos_sq_alpha * ((pow(req, 2.) - pow(rpol, 2.)) / pow(rpol, 2.));
	const double A = 1. + (usq / 16384.) * (4096. + usq * (-768. + usq * (320. - 175. * usq)));
	const double B = (usq / 1024.) * (256. + usq * (-128. + usq * (74. - 47. * usq)));
	const double delta_sig = B * sin_sigma * (cos2sigma + 0.25 * B * (cos_sigma * (-1. + 2. * pow(cos2sigma, 2.)) -
		(B / 6.) * cos2sigma * (-3. + 4. * pow(sin_sigma, 2.)) *
		(-3. + 4. * pow(cos2sigma, 2.))));

	//distance in m
	const double dis = rpol * A * (sigma - delta_sig);
	//angles in radians and deg
	const double alpha1 = atan2(cos(u2) * sin(lam) , cos(u1) * sin(u2) - sin(u1) * cos(u2) * cos(lam) ) ;
	const double alpha2 = atan2(cos(u1) * sin(lam) , -sin(u1) * cos(u2) + cos(u1) * sin(u2) * cos(lam) );
	const double alpha1_deg = 180. * alpha1 / PI;
	const double alpha2_deg = 180. * alpha2 / PI;


	//one of these asimuths is actually correct!
	/*
	UtilityFunctions::print("Distance = ");
	UtilityFunctions::print(dis);
	UtilityFunctions::print("Azimuth 1 = ");
	UtilityFunctions::print(alpha1);
	UtilityFunctions::print(alpha1_deg);
	UtilityFunctions::print("Azimuth 2 = ");
	UtilityFunctions::print(alpha2);
	UtilityFunctions::print(alpha2_deg);
	*/
	Array dis_and_angle{};
	dis_and_angle.push_back(dis);
	dis_and_angle.push_back(alpha1);

	return dis_and_angle;
}

//equations is indeterminate if the initial point is at the north or south pole
//if the initial azimuth is due east or west then the second equation is indeterminate 
//though if the second argument of atan2 is used these values are "handled correctly" - wikipedia
Array VincetyFunctions::calculate_coordinates(double latp, double longp, double distance, double azimuth) {
	//UtilityFunctions::print("Calculating Distance");
	//for now use a PI value

	// convert to radians
	latp = PI * latp / 180.0;
	longp = PI * longp / 180.0;
	//const double azimuth = azimuth_deg * PI/180;

	
	const double u1 = atan( (1. - flat) * tan(latp) ); //

	const double sigma1 = atan2( tan(u1) , cos(azimuth)); //azimuth is clock wise from north in the direction P1P2 produced??
	sin_alpha = cos(u1) * sin(azimuth); //
	cos_sq_alpha = 1. - pow(sin_alpha, 2.); //
	//const double alpha = asin(sin_alpha);
	const double usq = cos_sq_alpha * ((pow( req ,2.) - pow( rpol ,2.))/pow( rpol,2.)); //can use (1-sin^2 alpha) instead... //
	//UtilityFunctions::print(usq);
	
	const double A = 1. + (usq / 16384.0) * (4096.0 + usq * (-768.0 + usq * (320.0 - 175.0 * usq) ) ); //
	const double B = (usq / 1024.0) * (256.0 + usq * ( -128.0 + usq * (74.0 - 47.0 * usq)));  //

	//sigma = distance / (rpol * A); //angular seperation
	const double tol = pow(10., -12.);
	double diff = 1.;
	double delta_sigma;
	
	int iter_counter = 0;
	
	while (abs(diff) > tol) {
		two_sigma_m = 2. * sigma1 + sigma; //fixed a * to a +

		delta_sigma = B * sin(sigma) * (//
			cos(two_sigma_m) + B/4. * ( //
				cos(sigma) * (-1. + 2. * pow( cos(two_sigma_m), 2.) ) //
				- B/6. * cos(two_sigma_m) * (-3. + 4. * pow( sin(sigma), 2.) ) * (-3. + 4. * pow(cos(two_sigma_m), 2.) )//
				)
			);
		sigma_pre = sigma;//

		sigma = (distance /(rpol * A) ) + delta_sigma;//

		diff = abs(sigma_pre - sigma);//

		iter_counter += 1;
		if (iter_counter > 15) { diff = 0.00000000001; UtilityFunctions::print("LoopTimedOut"); }
	}
	
	double latc = atan2(
		sin(u1) * cos(sigma) + cos(u1) * sin(sigma) * cos(azimuth), //
		(1. - flat) * sqrt( pow(sin_alpha, 2.) + pow( sin(u1) * sin(sigma) - cos(u1) * cos(sigma) * cos(azimuth) , 2. ) )//
		);
	double lam = atan2( sin(sigma) * sin(azimuth), cos(u1) * cos(sigma) - sin(u1) * sin(sigma) * cos(azimuth) ); //can reuse the last factor of both equations for less computation //
	C = (flat / 16.) * cos_sq_alpha * (4. + flat * (4. - 3. * cos_sq_alpha ) ) ; //
	double lon = lam - (1. - C) * flat * sin_alpha * ( //
		sigma + C * sin(sigma) * (//
			cos(two_sigma_m) + C * cos(sigma) * (-1. + 2. * pow(cos(two_sigma_m) ,2.) ) //
				)
			); //difference in longtitudes
	double longc = lon + longp;

	/*
	UtilityFunctions::print("Coordinates are at:");
	UtilityFunctions::print(latc);
	UtilityFunctions::print(latc * 180. / PI);
	UtilityFunctions::print(longc);
	UtilityFunctions::print(longc* 180./PI);
	*/
	double alpha2 = atan2(sin_alpha, -sin(u1) * sin(sigma) + cos(u1) * cos(sigma) * cos(azimuth)); //still unsure about this
	/*
	UtilityFunctions::print("Back Azimuth:");
	UtilityFunctions::print(alpha2);
	UtilityFunctions::print(alpha2 * 180./PI);
	*/
	Array coordinates{};
	coordinates.push_back( latc *180. / PI );
	coordinates.push_back(longc * 180. / PI);
	//UtilityFunctions::print(coordinates);
	return coordinates;
}
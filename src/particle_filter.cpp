/*
 * particle_filter.cpp
 *
 *  Created on: Dec 12, 2016
 *      Author: Tiffany Huang
 */

#include <random>
#include <algorithm>
#include <iostream>
#include <numeric>
#include <math.h> 
#include <iostream>
#include <sstream>
#include <string>
#include <iterator>

#include "particle_filter.h"

using namespace std;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
	// TODO: Set the number of particles. Initialize all particles to first position (based on estimates of 
	//   x, y, theta and their uncertainties from GPS) and all weights to 1. 
	// Add random Gaussian noise to each particle.
	// NOTE: Consult particle_filter.h for more information about this method (and others in this file).
  default_random_engine gen;
  normal_distribution<double> dist_x(x, std[0]);
  normal_distribution<double> dist_y(y, std[1]);
  normal_distribution<double> dist_theta(theta, std[2]);
  for (int i=0; i<num_particles; ++i) {
    Particle p;
    p.x = dist_x(gen);
    p.y = dist_y(gen);
    p.theta = dist_theta(gen);
    p.weight = 1;
    particles.push_back(p);
    weights.push_back(1.0);
  }
  is_initialized = true;
}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
	// TODO: Add measurements to each particle and add random Gaussian noise.
	// NOTE: When adding noise you may find std::normal_distribution and std::default_random_engine useful.
	//  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
	//  http://www.cplusplus.com/reference/random/default_random_engine/
  default_random_engine gen;

  for (int i=0; i<num_particles; ++i) {
    particles[i].x += (velocity/yaw_rate) * 
                      (sin(particles[i].theta + (yaw_rate * delta_t)) - 
                       sin(particles[i].theta));
    normal_distribution<double> dist_x(particles[i].x, std_pos[0]);
    particles[i].x = dist_x(gen);

    particles[i].y += (velocity/yaw_rate) * 
                      (cos(particles[i].theta) -
                       cos(particles[i].theta + (yaw_rate * delta_t)));
    normal_distribution<double> dist_y(particles[i].y, std_pos[1]);
    particles[i].y = dist_y(gen);

    particles[i].theta += yaw_rate * delta_t;
    normal_distribution<double> dist_theta(particles[i].theta, std_pos[2]);
    particles[i].theta = dist_theta(gen);
  }
}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
	// TODO: Find the predicted measurement that is closest to each observed measurement and assign the 
	//   observed measurement to this particular landmark.
	// NOTE: this method will NOT be called by the grading code. But you will probably find it useful to 
	//   implement this method and use it as a helper during the updateWeights phase.

}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
		std::vector<LandmarkObs> observations, Map map_landmarks) {
	// TODO: Update the weights of each particle using a mult-variate Gaussian distribution. You can read
	//   more about this distribution here: https://en.wikipedia.org/wiki/Multivariate_normal_distribution
	// NOTE: The observations are given in the VEHICLE'S coordinate system. Your particles are located
	//   according to the MAP'S coordinate system. You will need to transform between the two systems.
	//   Keep in mind that this transformation requires both rotation AND translation (but no scaling).
	//   The following is a good resource for the theory:
	//   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
	//   and the following is a good resource for the actual equation to implement (look at equation 
	//   3.33. Note that you'll need to switch the minus sign in that equation to a plus to account 
	//   for the fact that the map's y-axis actually points downwards.)
	//   http://planning.cs.uiuc.edu/node99.html


  for (int k=0; k<particles.size(); ++k) {
    Particle& particle = particles[k];
    // px and py are map coordinates.
    double px = particle.x;
    double py = particle.y;
    double ptheta = particle.theta;
    particle.weight = 1.0;

    // For each particle, get observations around it.
    for (int i=0; i<observations.size(); ++i) {
      // ox and oy are vehicle coordinates.
      double ox = observations[i].x;
      double oy = observations[i].y;

      // TEST: should return 0, 5 according to 
      //       the class.
      // px = 4;
      // py = 5;
      // ox = 0;
      // oy = -4;
      // ptheta = -M_PI/2;

      // Convert observation x and y into map coordinates.
      double tx = px + ox * cos(ptheta) - oy * sin(ptheta);
      double ty = py + ox * sin(ptheta) + oy * cos(ptheta);
      // cout << "result: " << tx << ", " << ty << endl;

      
      //loop over number of landmarks and find the closest landmark:
      float min_d = sensor_range;
      int closest_landmark_id = -1;
      for (unsigned int l=0; l< map_landmarks.landmark_list.size(); ++l){

        //estimate pseudo range for each single landmark.
        //and the current observation:
        float dx = map_landmarks.landmark_list[l].x_f - tx;
        float dy = map_landmarks.landmark_list[l].y_f - ty;
        float d = sqrt(dx*dx + dy*dy);
        if (d < min_d) {
          min_d = d;
          closest_landmark_id = l;
        }
      }
      if (min_d < sensor_range) {
        // Set observation's id to the closest landmark.
        observations[i].id = closest_landmark_id;

        // Calculate gaussian probability of this observation.
        float lx = map_landmarks.landmark_list[closest_landmark_id].x_f;
        float ly = map_landmarks.landmark_list[closest_landmark_id].y_f;
        double a = pow(tx-lx, 2) / (2 * pow(std_landmark[0], 2));
        double b = pow(ty-ly, 2) / (2 * pow(std_landmark[1], 2));
        double prob = (1 / (2 * M_PI * std_landmark[0] * std_landmark[1])) * exp(-(a + b));
        particle.weight *= prob;
      }
      
    }
    weights[k] = particle.weight;
  }

  // Normalize weights of all particles.
  // NOT NEEDED - normalization is done in resample() function.
  // for (int k=0; k<particles.size(); ++k) {
  //   cout << "Before: " << particles[k].weight << endl;
  //   particles[k].weight /= total_weights;
  //   cout << "After: " << particles[k].weight << endl;
  // }
}

void ParticleFilter::resample() {
	// TODO: Resample particles with replacement with probability proportional to their weight. 
	// NOTE: You may find std::discrete_distribution helpful here.
	//   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
  std::random_device dev;
  std::mt19937 gen(dev());
  std::discrete_distribution<int> d(weights.begin(), weights.end());

  std::vector<Particle> resampled;
  for (int i=0; i < particles.size(); i++) 
  {
      int rnd = d(gen);
      resampled.push_back(particles[rnd]);
  }
  particles = resampled;
}

Particle ParticleFilter::setAssociations(Particle particle, std::vector<int> associations, std::vector<double> sense_x, std::vector<double> sense_y)
{
	//particle: the particle to assign each listed association, and association's (x,y) world coordinates mapping to
	// associations: The landmark id that goes along with each listed association
	// sense_x: the associations x mapping already converted to world coordinates
	// sense_y: the associations y mapping already converted to world coordinates

	//Clear the previous associations
	particle.associations.clear();
	particle.sense_x.clear();
	particle.sense_y.clear();

	particle.associations= associations;
 	particle.sense_x = sense_x;
 	particle.sense_y = sense_y;

 	return particle;
}

string ParticleFilter::getAssociations(Particle best)
{
	vector<int> v = best.associations;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<int>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseX(Particle best)
{
	vector<double> v = best.sense_x;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}
string ParticleFilter::getSenseY(Particle best)
{
	vector<double> v = best.sense_y;
	stringstream ss;
    copy( v.begin(), v.end(), ostream_iterator<float>(ss, " "));
    string s = ss.str();
    s = s.substr(0, s.length()-1);  // get rid of the trailing space
    return s;
}

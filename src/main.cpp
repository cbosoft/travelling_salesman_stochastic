#include <iostream>
#include <ios>
#include <fstream>
#include <vector>
#include <random>
#include <cmath>
#include <map>

enum LOC_TYPE { LOC_CITY, LOC_BRIDGE };
class Location {
  private:
    double x, y;
    int type, index;
  public:
    Location(){}
    Location(int type, double x, double y, int index) {
      this->type = type;
      this->x = x;
      this->y = y;
      this->index = index;
    }

    double get_x() const { return this->x; }
    double get_y() const { return this->y; }
    int get_index() const { return this->index; }
    void get_values(double &x, double &y, int &index) const {
      x = this->x;
      y = this->y;
      index = this->index;
    }

    bool is_city() const {
      return this->type == LOC_CITY;
    }

    void print() const {
      //std::cout << (this->is_city() ? "C " : "B ") << this->x << " " << this->y << std::endl;
      std::cout << this->x << " " << this->y << std::endl;
    }

};

class City : public Location {
  public:
    City(){}
    City (double x, double y, int index) : Location (LOC_CITY, x, y, index) {}
};
class Bridge : public Location {
  public:
    Bridge(){}
    Bridge (double x, double y, int index) : Location (LOC_BRIDGE, x, y, index) {}
};

bool within_range(double r1, double r2, double v) {
  double upper = r2, lower = r1;

  if (r1 > r2) {
    upper = r1;
    lower = r2;
  }
  else if (r1 == r2) {
    std::cerr << "range is zero" << std::endl;
  }

  return (lower < v) && (v < upper);
}


class River {
  private:
    double m, c;
    bool set;
  public:
    River(){
      this->set = true;
    }
    River(double m, double c) 
    {
      this->m = m;
      this->c = c;
      this->set = true;
    }

    bool are_same_side_p(const double& x1, const double& y1, const double& x2, const double& y2) {
      double mp, cp;
      mp = (y2 - y1) / (x2 - x1);
      cp = y1 - (mp*x1);

      if (this->m == mp) {
        if (this->c == cp) {
          std::cerr << "cities lie exactly on river" << std::endl;
        }
        return true;
      }

      double x = (cp - this->c) / (this->m - mp);
      double y = (mp*x)+cp;

      return (within_range(x1, x2, x) && within_range(y1, y2, y));
    }
    operator bool() const {
      return this->set;
    }

};




typedef std::vector<Location *> Route;

void print_route(const Route& route) {
  for (auto it = route.begin(); it != route.end(); ++it)
    (*it)->print();
}



double distance_two_coords(double x1, double y1, double x2, double y2) {
  return std::pow(std::pow(x1-x2, 2.0) + std::pow(y1-y2, 2.0), 0.5);
}


class StochOpt {
  private:
    std::vector<std::pair<int, int>> swap_history;
    Route route;
    Route best_route;
    double best_distance;
    Route working_route;
    std::vector<Bridge> bridges;
    std::random_device rd;
    std::mt19937 randgen;
    std::uniform_int_distribution<> index_dist;
    std::uniform_real_distribution<> prob_dist;
    //std::map<std::pair<int, int>, double> dist_lookup;
    double **dist_lookup;
    River river;

    int initial_number_swaps, number_steps, number_shuffles;
    double initial_probability, probability_rate;

  public:
    StochOpt(){
      this->randgen = std::mt19937(this->rd());
      best_distance = 1e10;
    };

    StochOpt(Route route) : StochOpt() { 
      this->route = route;
      this->working_route = route;
      this->init_lookuptable();
      this->index_dist = std::uniform_int_distribution<>(0, route.size()-1);
      this->prob_dist = std::uniform_real_distribution<>(0.0, 1.0);
      this->initial_number_swaps = 3*this->route.size()/2;
      this->number_steps = 100;
      this->number_shuffles = 10000;
      this->initial_probability = 0.9;
      this->probability_rate = 0.9;
    };

    void swap(int left, int right) {
      Location *temp = this->working_route[left];
      this->working_route[left] = this->working_route[right];
      this->working_route[right] = temp;
    }

    int get_rand_int() {
      return this->index_dist(this->randgen);
    }
    double get_rand_double() {
      return this->prob_dist(this->randgen);
    }

    void shuffle(int n = 1) {
      int left = 0, right = 0;
      for (int i = 0; i < n; i++) {

        // get two distinct indices
        left = this->get_rand_int();
        do {
          right = this->get_rand_int();
        } while (right == left);

        // swap and add to history
        this->swap(left, right);
        this->swap_history.push_back(std::make_pair(left, right));

      }
    }

    void unshuffle(int n = 1) {
      std::pair<int, int> swap;
      for (int i = 0; i < n; i++) {

        // get last swap from history
        swap = this->swap_history.back();

        // redo the swap, remove from history
        this->swap(swap.first, swap.second);
        this->swap_history.pop_back();
      }
    }

    void clear() {
      this->swap_history.clear();
    }

    // TODO: river!
    bool crosses_river_p(const double& x1, const double& y1, const double& x2, const double& y2) {
      if (!this->river)
        return false;
      
      return this->river.are_same_side_p(x1, y1, x2, y2);
    }

    void add_bridge(Bridge bridge) {
      this->bridges.push_back(bridge);
    }

    void add_river(River river) {
      this->river = river;
    }

    Bridge get_best_bridge(const double& xa, const double& ya, const double& xb, const double& yb) {
      if (this->bridges.size() == 1)
        return this->bridges.front();

      Bridge bridge_best;
      double distance_best = 1e10, distance, xbridge, ybridge;
      int bridge_index;
      for (const auto& bridge : this->bridges) {
        bridge.get_values(xbridge, ybridge, bridge_index);
        distance = distance_two_coords(xa, ya, xbridge, ybridge) + distance_two_coords(xbridge, ybridge, xb, yb);
        if (distance < distance_best) {
          distance_best = distance_best;
          bridge_best = bridge;
        }
      }

      return bridge_best;
    }

    void init_lookuptable() {
      double distance = 0.0;
      const int routelen = this->route.size();
      this->dist_lookup = new double*[routelen];
      
      for (int i = 0; i < routelen; i++) {
        this->dist_lookup[i] = new double[routelen];
      }
      
      double xa, xb, ya, yb;
      int a, b;
      for (const auto& city_a : this->route) {
        for (const auto& city_b : this->route) {
          if (city_a->get_index() == city_b->get_index()) continue;
          city_a->get_values(xa, ya, a);
          city_b->get_values(xb, yb, b);
          if (this->crosses_river_p(xa, ya, xb, yb)) {
            Bridge bridge = this->get_best_bridge(xa, ya, xb, yb);
            double xbridge, ybridge;
            int bridge_index;
            bridge.get_values(xbridge, ybridge, bridge_index);
            distance = distance_two_coords(xa, ya, xbridge, ybridge) + distance_two_coords(xbridge, ybridge, xb, yb);
            std::cerr << "crosses river" << std::endl;
          }
          else {
            distance = distance_two_coords(xa, ya, xb, yb);
          }
          this->dist_lookup[a][b] = distance;
        }
      }
    }

    double get_distance(int a, int b) {
      return this->dist_lookup[a][b];
    }

    double get_route_distance(const Route& route) {
      double total = 0.0;
      int len = route.size(), ni = 0;
      for (int i = 0; i < len; i++) {
        ni = (i == (len-1)) ? 0 : i+1;
        total += get_distance( route[i]->get_index(), route[ni]->get_index() );
      }
      return total;
    }

    double get_working_route_distance() { return this->get_route_distance(this->working_route); }
    double get_working_best_distance() { return this->get_route_distance(this->best_route); }

    void optimise() {

      // initial values
      double distance = get_working_route_distance(), dist_new = 0.0, delta_dist = 0.0;
      double probability = this->initial_probability;
      int number_swaps = this->initial_number_swaps;

      this->best_route = this->working_route;
      this->best_distance = distance;

      for (int j = 0; j < this->number_steps; j++) {

        //std::cerr << "probability: " << probability << " dist: " << distance << " number_swaps: " << number_swaps << std::endl;
        for (int i = 0; i < this->number_shuffles; i++) {
          this->shuffle(number_swaps);

          dist_new = get_working_route_distance();
          delta_dist = dist_new - distance;

          if (delta_dist <= 0.0) {
            distance = dist_new;
          }
          else if (this->get_rand_double() < probability ) {
            distance = dist_new;
          }
          else {
            this->unshuffle(number_swaps);
          }

          if (distance < this->best_distance) {
            this->best_route = this->working_route;
            this->best_distance = distance;
          }

        }

        probability *= this->probability_rate;
        if (number_swaps > 1) {
          number_swaps -= 1;
        }
        else {
          number_swaps = 1;
        }

      }

      print_route(this->best_route);
      std::cerr << this->best_distance << std::endl;
    }
};




Route get_cities(const char *path)
{
  std::fstream cities_file(path);
  
  Route route;
  City *pair;
  double x, y;
  int i = 0;

  while (cities_file >> x >> y) {
    pair = new City(x, y, i++);
    route.push_back(pair);
  }
  
  return route;
}





int main(int argc, const char *argv[])
{
  const char* cities_file = "cities.txt";
  if (argc > 1)
    cities_file = argv[1];


  StochOpt opt(get_cities(cities_file));
  opt.add_river(River(0, 0.5));
  opt.add_bridge(Bridge(0.5, 0.5, 0));
  opt.optimise();


  // not delete'ing new'd memory: this is leaky.
}

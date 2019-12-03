#include <iostream>
#include <ios>
#include <fstream>
#include <vector>
#include <random>
#include <cmath>

typedef std::pair<double, double> City;
typedef std::vector<City *> CityList;

void print_cities(CityList *cities) {
  for (auto it = cities->begin(); it != cities->end(); ++it)
    std::cout << (*it)->first << " " << (*it)->second << std::endl;
}


class Shuffler {
  private:
    std::vector<std::pair<int, int>> shuffle_history;
    CityList *cities;
    std::random_device rd;
    std::mt19937 randgen;
    std::uniform_int_distribution<> index_dist;
    std::uniform_real_distribution<> prob_dist;
  public:
    Shuffler(){
      this->randgen = std::mt19937(this->rd());
    };
    Shuffler(CityList *cities) : Shuffler() { 
      this->cities = cities; 
      this->index_dist = std::uniform_int_distribution<>(0, cities->size()-1);
      this->prob_dist = std::uniform_real_distribution<>(0.0, 1.0);
    };

    void swap(int left, int right) {
      City *temp = (*this->cities)[left];
      (*this->cities)[left] = (*this->cities)[right];
      (*this->cities)[right] = temp;
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
        this->shuffle_history.push_back(std::make_pair(left, right));

      }
    }

    void unshuffle(int n = 1) {
      std::pair<int, int> swap;
      for (int i = 0; i < n; i++) {

        // get last swap from history
        swap = this->shuffle_history.back();

        // redo the swap, remove from history
        this->swap(swap.first, swap.second);
        this->shuffle_history.pop_back();
      }
    }

    void clear() {
      this->shuffle_history.clear();
    }
};




CityList *get_cities(const char *path)
{
  std::fstream cities_file(path);
  
  CityList *cities = new std::vector<std::pair<double, double> *>();
  City *pair = NULL;
  double x, y;

  while (cities_file >> x >> y) {
    pair = new City(x, y);
    cities->push_back(pair);
  }
  
  return cities;
}

double get_distance(City *a, City *b) {
  return std::pow((a->first - b->first), 2.0) + std::pow((a->second - b->second), 2.0);
}


double get_route_distance(CityList *cities) {
  double total = 0.0;
  int len = cities->size(), ni = 0;
  for (int i = 0; i < len; i++) {
    ni = (i == (len-1)) ? 0 : i+1;
    total += get_distance( (*cities)[i], (*cities)[ni] );
  }
  return total;
}

// TODO:
// table of precomputed distances
// move functions to class methods of CityList class





int main(int argc, const char *argv[])
{
  const char* cities_file = "cities.txt";
  if (argc > 1)
    cities_file = argv[1];

  auto cities = get_cities(cities_file);

  Shuffler shuffler(cities);
  double beta = 1;
  int nshuffles = 4*cities->size();
  int ndecr = 15;
  int dshuffles = nshuffles / (ndecr - 1);
  double dist = get_route_distance(cities), dist_new = 0.0;//, delta_dist = 0.0;

  for (int j = 0; j < ndecr; j++) {

    for (int i = 0; i < 1000; i++) {
      std::cerr << "nshuff: " << nshuffles << " dist: " << dist << " ";
      shuffler.shuffle(nshuffles);

      dist_new = get_route_distance(cities);
      //delta_dist = std::abs(dist-dist_new);

      if (dist_new < dist) {
        std::cerr << "distance " << dist_new << " is better than " << dist << std::endl;
        dist = dist_new;
      }
      else if (shuffler.get_rand_double() < std::exp(-beta) ) {
        std::cerr << "random chance " << beta << std::endl;
        dist = dist_new;
      }
      else {
        std::cerr << "undoing" << std::endl;
        shuffler.unshuffle(nshuffles);
      }

      shuffler.clear();

    }

    beta *= 2.0;
    nshuffles -= dshuffles;

  }

  print_cities(cities);
  std::cerr << "Optimal route length: " << dist << std::endl;
}

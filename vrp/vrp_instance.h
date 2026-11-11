
#ifndef _VRPINSTANCE_H
#define _VRPINSTANCE_H

#include <string>
#include <vector>


struct instance_data_struct {
  std::vector<std::vector<double>> distanceMatrix;
  std::vector<double> distanceWarehouses;
  std::vector<int> demands;
  int nbCustomers;
  int truckCapacity;
};


class VrpInstance{
  private:
    /** Capacity of the trucks */
    int vehicleCapacity;
    /** Number of customers */
    int nbCustomers;
    /** Distance matrix | distance between customers */
    std::vector<std::vector<double>> distanceMatrix;
    /** Distance to depot distance between customers and the depot */
    std::vector<double> distanceWarehouses;
    /** Demand on each node */
    std::vector<int> demands;

    bool silence;

    double computeDist(int xi, int xj, int yi, int yj);
    //void computeDistanceMatrix(int depotX, int depotY, const std::vector<int>& customersX, const std::vector<int>& customersY);
    std::vector<std::vector<double>> computeDistanceMatrix(int nbCustomers, const std::vector<float>& customersX, const std::vector<float>& customersY);
    std::vector<double> computeDistanceToWarehouse(int nbCustomers, int depotX, int depotY, const std::vector<float>& customersX, const std::vector<float>& customersY);

  public:
    VrpInstance();
    ~VrpInstance(){};

    /**  Read write privates attributes : */
    int getNbCustomers();
    void setNbCustomers(int nbCustomers);

    std::vector<int> getDemands();
    void setDemands(std::vector<int> demands);

    std::vector<std::vector<double>> getDistanceMatrix();
    void setDistanceMatrix(std::vector<std::vector<double>> distanceMatrix);

    std::vector<double> getDistanceWarehouses();
    void setDistanceWarehouses(std::vector<double> distanceWarehouses);

    int getVehicleCapacity();
    void setVehicleCapacity(int nbCustomers);

    /**  Read Data from a file : */
    bool readDataFromFile(const std::string _fileName);

    void setSilence(bool s);

    double computeSolution(std::vector<std::vector<int>> & solution);
};


#endif

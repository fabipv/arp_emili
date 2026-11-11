#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cmath>
#include <stdexcept>

#include "vrp_instance.h"

using namespace std;

/** Constructors and Destroyer */
VrpInstance::VrpInstance()
{
  silence = false;
}


/** Get and Set Methods */
int VrpInstance::getNbCustomers(){ return nbCustomers; }
void VrpInstance::setNbCustomers(int nbCustomers){ this->nbCustomers = nbCustomers; }

vector<int> VrpInstance::getDemands(){ return demands; }
void VrpInstance::setDemands( vector<int> demands ){ this->demands = demands; }

vector<vector<double>> VrpInstance::getDistanceMatrix(){ return distanceMatrix; }
void VrpInstance::setDistanceMatrix( vector<vector<double>> distanceMatrix ){ this->distanceMatrix = distanceMatrix; }

vector<double> VrpInstance::getDistanceWarehouses(){ return distanceWarehouses; }
void VrpInstance::setDistanceWarehouses( vector<double> distanceWarehouses ){ this->distanceWarehouses = distanceWarehouses; }


/**  Read the instance from file : */
/** The input files follow the "Augerat" format. */
bool VrpInstance::readDataFromFile(const std::string _fileName)
{
  bool everythingOK = true;
  std::ifstream infile(_fileName.c_str(), std::ios_base::in);

  if (infile.is_open() == false) {
    if(!silence)
    std::cout    << "ERROR. file:vrp_instance.cpp, method:readDataFromFile, "
            << "error while opening file " << _fileName << std::endl;
    everythingOK = false;
    return everythingOK;
  }

  if(!silence){
    std::cout << "File " << _fileName << " is now open" << std::endl;
    std::cout << "Starting to read data..." << std::endl;
  }

  string str;
  char *pch;
  char* line;

  int nbNodes;

  while (true) {
      getline(infile, str);
      line = strdup(str.c_str());
      pch = strtok(line, " :");
      if (strcmp(pch, "DIMENSION") == 0) {
          pch = strtok(NULL, " :");
          nbNodes = atoi(pch);
          nbCustomers = nbNodes - 1;

      } else if (strcmp(pch, "CAPACITY") == 0) {
          pch = strtok(NULL, " :");
          vehicleCapacity = atoi(pch);
          //std::cout << "Capacidad :" << truckCapacity << std::endl;

      } else if (strcmp(pch, "EDGE_WEIGHT_TYPE") == 0) {
          pch = strtok(NULL, " :");
          if (strcmp(pch, "EUC_2D") != 0) {
              throw std::runtime_error("***Only Edge Weight Type EUC_2D is supported");
          }
      } else if (strcmp(pch, "NODE_COORD_SECTION") == 0) {
          break;
      }
      free(line);
  }


  vector<float> customersX(nbCustomers);
  vector<float> customersY(nbCustomers);
  //std::cout << infile.rdbuf() << std::endl;
  int depotX, depotY;
  for (int n = 1; n <= nbNodes; n++) {
      int id;
      infile >> id;
      //std::cout << id <<std::endl;
      //std::cout << n <<std::endl;
      //id = n;
      if (id != n) {
          throw std::runtime_error("***Unexpected index");
      }
      if (n == 1) {
          infile >> depotX;
          infile >> depotY;
          //std::cout <<"depotX "<< depotX<<std::endl;
          //std::cout <<"depotY "<< depotY<<std::endl;
      } else {
          // -2 because orginal customer indices are in 2..nbNodes
          infile >> customersX[n-2];
          infile >> customersY[n-2];
	  //std::cout <<"costumerX "<< customersX[n-2]<<std::endl;
	  //std::cout <<"costumerY "<< customersY[n-2]<<std::endl;
      }
      //std::cout << customersX <<std::endl;
      //std::cout << customersY <<std::endl;
      //std::cout << std::endl;
  }
  // Compute distance matrix
  distanceMatrix = computeDistanceMatrix(nbCustomers, customersX, customersY);
  distanceWarehouses = computeDistanceToWarehouse(nbCustomers, depotX, depotY, customersX, customersY);

  getline(infile, str); // End the last line
  getline(infile, str);
  line = strdup(str.c_str());
  pch = strtok(line, " :");
  if (strcmp(pch, "DEMAND_SECTION") != 0) {
      throw std::runtime_error("***Expected keyword DEMAND_SECTION");
  }
  free(line);
  //std::cout <<"llego al segundo" <<std::endl;
  demands.resize(nbCustomers);
  for (int n = 1; n <= nbNodes; n++) {
      int id;
      infile >> id;


      if (id != n) {
          throw std::runtime_error("***Unexpected index");
      }
      int demand;
      infile >> demand;
      if (n == 1) {
          if (demand != 0) {
              throw std::runtime_error("***Demand for depot should be O");
          }
      } else {
          // -2 because orginal customer indices are in 2..nbNodes
          demands[n-2] = demand;
      }
  }

  getline(infile, str); // End the last line
  getline(infile, str);
  line = strdup(str.c_str());
  pch = strtok(line, " :");
  if (strcmp(pch, "DEPOT_SECTION") != 0) {
    
      throw std::runtime_error("***Expected keyword DEPOT_SECTION");
  }
  free(line);

  int warehouseId;
  infile >> warehouseId;
  if (warehouseId != 1) {
      throw std::runtime_error("***Warehouse id is supposed to be 1");
  }

  int endOfDepotSection;
  infile >> endOfDepotSection;
  if (endOfDepotSection != -1) {
      throw std::runtime_error("***Expecting only one warehouse, more than one found");
  }

  infile.close();
  cout << "Distance matrix between customers"<< endl;
  /*if(!silence){
    cout << endl << "INSTANCE INFO" << endl << "Number of customers: "<< nbCustomers <<endl;
  	cout << "Vehicle capacity: "<< vehicleCapacity <<endl;
  	cout << "\n";
  	cout << "Distance matrix between customers"<< endl;
  	for(int i=0;i<distanceMatrix.size();i++){
        for(int j=0;j<distanceMatrix[i].size();j++){
            cout << distanceMatrix[i][j] << " ";
        }
    cout<<endl;
    }
    cout << "\n";
  	cout << "Distance between each customer and the warehouse "<< endl;
  	for(int i=0;i<distanceWarehouses.size();i++){
        cout << distanceWarehouses[i] << " ";
    cout<<endl;
    }
    cout << "\n";
  	cout << "Demand of each customer"<< endl;
  	for(int i=0;i<demands.size();i++){
        cout << demands[i] << " ";
    cout<<endl;
    }
    std::cout << std::endl;
  }*/

  return true;
}


/** Computes the distance matrix */
std::vector<std::vector<double>> VrpInstance::computeDistanceMatrix(int nbCustomers, const std::vector<float>& customersX, const std::vector<float>& customersY)
{
  std::vector<std::vector<double>> distanceMatrix;
  distanceMatrix.resize(nbCustomers);

  for (int i = 0; i < nbCustomers; i++) {
      distanceMatrix[i].resize(nbCustomers);
  }
  for (int i = 0; i < nbCustomers; i++) {
      distanceMatrix[i][i] = 0;
      for (int j = i + 1; j < nbCustomers; j++) {
          double distance = this->computeDist(customersX[i], customersX[j], customersY[i], customersY[j]);
          distanceMatrix[i][j] = distance;
          distanceMatrix[j][i] = distance;
      }
  }

  return distanceMatrix;
}

std::vector<double>VrpInstance::computeDistanceToWarehouse(int nbCustomers, int depotX, int depotY, const std::vector<float>& customersX, const std::vector<float>& customersY)
{
  std::vector<double> distanceWarehouses;
  distanceWarehouses.resize(nbCustomers);
  for(int i = 0; i < nbCustomers; ++i) {
      distanceWarehouses[i] = this->computeDist(depotX, customersX[i], depotY, customersY[i]);
      //std::cout << "Distancia del cliente" << i+1 << " al deposito es de:" << distanceWarehouses[i] << std::endl;
  }
  return distanceWarehouses;
}

double VrpInstance::computeDist(int xi, int xj, int yi, int yj)
{
  double dist = sqrt(pow((double) xi - xj, 2) + pow((double) yi - yj, 2));
  //return floor(exactDist + 0.5);
  return dist;
}

void VrpInstance::setSilence( bool s )
{
  this->silence = s;
}


int VrpInstance::getVehicleCapacity()
{
  return vehicleCapacity;
}

void VrpInstance::setVehicleCapacity( int vehicleCapacity )
{
  this->vehicleCapacity = vehicleCapacity;
}

double VrpInstance::computeSolution(std::vector < vector<int> > &solution)
{
    double computedValue = 0;

    for(int i=0; i<solution.size(); i++){
      computedValue = computedValue + distanceWarehouses[solution[i][0]];

      if(solution[i].size() > 0){
        computedValue = computedValue + distanceWarehouses[solution[i][solution[i].size()-1]];
      }
      for(int j=0; j<solution[i].size()-1; j++){
        computedValue = computedValue + distanceMatrix[solution[i][j]][solution[i][j+1]];
      }
    }

    return computedValue;
}

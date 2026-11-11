#include "vrp.h"

#include <cstdlib>
#include <ctime>
//#include <chrono>

//-----------------
#include <climits>
#include <string>
#include <sstream>
#include <assert.h>
#include <algorithm>
#include <stack>
#include <limits>
#include <vector>
//----------------


void notyet()
{
    std::cout << "Not implemented yet \n";
    //std::exit(-5);
}



/*----------------------------------------------------------------------------------------------------------------*
 *--------------------------------------------------PROBLEM-------------------------------------------------------*
 *----------------------------------------------------------------------------------------------------------------*/


double emili::vrp::VrpProblem::calcObjectiveFunctionValue(emili::Solution &solution)
{
    emili::vrp::VrpSolution& s = dynamic_cast<emili::vrp::VrpSolution&> (solution);
    return computeObjectiveFunction(s.getSolution());
}

double emili::vrp::VrpProblem::evaluateSolution(emili::Solution &solution)
{
    emili::vrp::VrpSolution& s = dynamic_cast<emili::vrp::VrpSolution&> (solution);
    double p = computeObjectiveFunction(s.getSolution());
    solution.setSolutionValue(p);
    return p;
}

double emili::vrp::VrpProblem::computeObjectiveFunction(std::vector < std::vector<int> > &solution)
{
    return this->computeObjectiveFunction(solution);
}

int emili::vrp::VrpProblem::getNcustomers()
{
    return instance.getNbCustomers();
}

int emili::vrp::VrpProblem::getVehicleCapacity()
{
    return instance.getVehicleCapacity();
}

std::vector<int> emili::vrp::VrpProblem::getDemands()
{
    return instance.getDemands();
}

std::vector<std::vector<double>> emili::vrp::VrpProblem::getDistanceMatrix()
{
    return instance.getDistanceMatrix();
}

std::vector<double> emili::vrp::VrpProblem::getDistanceWarehouses()
{
    return instance.getDistanceWarehouses();
}

double emili::vrp::CVRP::computeObjectiveFunction(std::vector < std::vector<int> > &solution)
{
  return instance.computeSolution(solution);  //ojo aqui
}

/*----------------------------------------------------------------------------------------------------------------*
 *----------------------------------------------INITIAL SOLUTION--------------------------------------------------*
 *----------------------------------------------------------------------------------------------------------------*/


emili::Solution* emili::vrp::VrpInitialSolution::generateEmptySolution() 
{
    std::vector< std::vector< int > > sol;
    double maxValue = std::numeric_limits<double>::max();
    std::vector<int> capacity;
    emili::vrp::VrpSolution*  em =  new emili::vrp::VrpSolution(maxValue, sol, capacity);
    return em;
}

emili::Solution* emili::vrp::VrpInitialSolution::generateSolution()
{
    return generate();
}

emili::Solution* emili::vrp::VrpDetInitialSolution::generate()
{
  std::vector < std::vector<int> > solution;
  std::vector <int> used_capacities;
  
  std::vector<int> customers(pis.getNcustomers());
  std::vector<int> demands = pis.getDemands();
  
  int vehicle_capacity = pis.getVehicleCapacity();
  int used = 0;
  
  std::vector<int> currentVehicleSolution;

  for(int i=0; i<customers.size(); i++){
    customers[i] = i;
  }

  int current_customer = 0;
  int current_vehicle = 0;
  used = 0;
  while (customers.size() > 0){
    if ((used + demands[current_customer]) > vehicle_capacity) {
      //new truck required
      solution.push_back(currentVehicleSolution);
      currentVehicleSolution.clear();
      current_vehicle++;
      used_capacities.push_back(used);
      used = 0;
    }
    //add customer
    currentVehicleSolution.push_back(current_customer);
    //increase used capacity
    used = used + demands[current_customer];
    //remove costumer
    customers.erase(customers.begin());
    current_customer++;
  }
  solution.push_back(currentVehicleSolution);
  used_capacities.push_back(used);
  
  VrpSolution* s = new VrpSolution(pis.computeObjectiveFunction(solution), solution, used_capacities);
  
  pis.evaluateSolution(*s);
  s->isFeasible();
  return s;
}

emili::Solution* emili::vrp::VrpRandomInitialSolution::generate()
{
  std::vector < std::vector<int> > solution;
  std::vector <int> used_capacities;
  
	std::vector<int> customers(pis.getNcustomers());
  std::vector<int> demands = pis.getDemands();
  
	int used = 0;
	int vehicle_capacity = pis.getVehicleCapacity();
	
	std::vector<int> currentVehicleSolution;

	for(int i =0; i<customers.size(); i++){	//vector with all the pos. Example  v = [ 0, 1, 2, 3 ..... (nbCustomers -1) ]
		customers[i] = i;
	}
  
  int rand_index = 0;
	int current_customer = 0;
	int current_vehicle = 0;
	int current_index = 0;
	
	while (customers.size() > 0){
	  rand_index = emili::generateRandomNumber() % customers.size();
		//rand_index = rand() % customers.size();
		current_customer = customers[rand_index];
		
		if ((used + demands[current_customer]) > vehicle_capacity) {
		  //new truck required
		  solution.push_back(currentVehicleSolution);
		  currentVehicleSolution.clear();
		  used_capacities.push_back(used);
		  current_vehicle++;
		  used = 0; 
		  current_index = 0;
		}
		
		//add customer
		currentVehicleSolution.push_back(current_customer);
		current_index++;
		//reduce capacity
		used = used + demands[current_customer];
  	customers.erase(customers.begin() + rand_index);
	}
	solution.push_back(currentVehicleSolution);
	used_capacities.push_back(used);
	
  VrpSolution* s = new VrpSolution(pis.computeObjectiveFunction(solution), solution, used_capacities);
  pis.evaluateSolution(*s);
  
  return s;
}


emili::Solution* emili::vrp::VrpNNInitialSolution::generate(){
 	std::vector < std::vector<int> > solution;
 	std::vector <int> used_capacities;
 	std::vector<int> customers(pis.getNcustomers());
 	std::vector<int> demands = pis.getDemands();
	int used = 0;
	int vehicle_capacity = pis.getVehicleCapacity();
	std::vector<int> currentVehicleSolution;
	std::vector<std::vector<double>> DistanceMatrix = pis.getDistanceMatrix();
	std::vector<double> WarehousesDistance = pis.getDistanceWarehouses();

	int current_customer = 0;
	int current_vehicle = 0;
	int current_index = 0;

	for(int i=0; i<customers.size(); i++){
		customers[i] = i;
	}

	while(customers.size() > 0){

		if(currentVehicleSolution.size() == 0){
			current_customer = customers[0];
			for(int i = 0; i < customers.size(); i++){
				//if(WarehousesDistance[current_customer] > WarehousesDistance[customers[i]] + (emili::generateRealRandomNumber())*100 ){
				if(WarehousesDistance[current_customer] > WarehousesDistance[customers[i]] ){
					current_customer = customers[i];
					current_index = i;
				}
			}
		}
		else{
			current_customer = customers[0];
			for(int j = 0; j < customers.size(); j++){
				if(current_customer != customers[j]){
					if(DistanceMatrix[ currentVehicleSolution[currentVehicleSolution.size()-1]  ][current_customer] > DistanceMatrix[currentVehicleSolution[currentVehicleSolution.size()-1]][customers[j]] ){
						current_customer = customers[j];
						current_index = j;
					}
				}
			}
		}
    if( (used + demands[current_customer]) > vehicle_capacity){
      solution.push_back(currentVehicleSolution);
      currentVehicleSolution.clear();
      used_capacities.push_back(used);
      current_vehicle++;
      used = 0;
    }
    else{
      currentVehicleSolution.push_back(current_customer);
      customers.erase(customers.begin()+current_index); //podria romperse sin poner el -1
      used += demands[current_customer];

    }
		current_index = 0;
		current_customer = 0;

	}
	solution.push_back(currentVehicleSolution);
	used_capacities.push_back(used);

	for(int i = 0; i < solution.size();i++){
		for(int j = 0; j < solution[i].size(); j++){
			std::cout <<solution[i][j]<<" ";
		}
		std::cout <<std::endl;
	}
	std::cout << "la solucion fitness es"<< std::endl;
	std::cout << pis.computeObjectiveFunction(solution) << std::endl;
	std:: cout << std::endl;
	VrpSolution* s = new VrpSolution(pis.computeObjectiveFunction(solution), solution, used_capacities);
 	pis.evaluateSolution(*s);

	return s;
}

emili::Solution* emili::vrp::VrpFNInitialSolution::generate(){
 	std::vector < std::vector<int> > solution;
 	std::vector <int> used_capacities;
 	std::vector<int> customers(pis.getNcustomers());
 	std::vector<int> demands = pis.getDemands();
	int used = 0;
	int vehicle_capacity = pis.getVehicleCapacity();
	std::vector<int> currentVehicleSolution;
	std::vector<std::vector<double>> DistanceMatrix = pis.getDistanceMatrix();
	std::vector<double> WarehousesDistance = pis.getDistanceWarehouses();

	int current_customer = 0;
	int current_vehicle = 0;
	int current_index = 0;

	for(int i=0; i<customers.size(); i++){
		customers[i] = i;
	}

	while(customers.size() > 0){

		if(currentVehicleSolution.size() == 0){
			current_customer = customers[0];
			for(int i = 0; i < customers.size(); i++){
				if(WarehousesDistance[current_customer] < WarehousesDistance[customers[i]] ){
					current_customer = customers[i];
					current_index = i;
				}
			}
		}
		else{
			current_customer = customers[0];
			for(int j = 0; j < customers.size(); j++){
				if(current_customer != customers[j]){
					if(DistanceMatrix[ currentVehicleSolution[currentVehicleSolution.size()-1]  ][current_customer] < DistanceMatrix[currentVehicleSolution[currentVehicleSolution.size()-1]][customers[j]]){
						current_customer = customers[j];
						current_index = j;
					}
				}
			}
		}
    if( (used + demands[current_customer]) > vehicle_capacity){
      solution.push_back(currentVehicleSolution);
      currentVehicleSolution.clear();
      used_capacities.push_back(used);
      current_vehicle++;
      used = 0;
    }
    else{
      currentVehicleSolution.push_back(current_customer);
      customers.erase(customers.begin()+current_index); //podria romperse sin poner el -1
      used += demands[current_customer];

    }
		current_index = 0;
		current_customer = 0;

	}
	solution.push_back(currentVehicleSolution);
	used_capacities.push_back(used);

	for(int i = 0; i < solution.size();i++){
		for(int j = 0; j < solution[i].size(); j++){
			std::cout <<solution[i][j]<<" ";
		}
		std::cout <<std::endl;
	}
	std::cout << "la solucion fitness es"<< std::endl;
	std::cout << pis.computeObjectiveFunction(solution) << std::endl;
	std:: cout << std::endl;
	VrpSolution* s = new VrpSolution(pis.computeObjectiveFunction(solution), solution, used_capacities);
 	pis.evaluateSolution(*s);

	return s;
}

emili::Solution* emili::vrp::VrpIMInitialSolution::generate(){
 	std::vector < std::vector<int> > solution;
 	std::vector <int> used_capacities;
 	std::vector<int> customers(pis.getNcustomers());
 	std::vector<int> demands = pis.getDemands();
	int used = 0;
	int vehicle_capacity = pis.getVehicleCapacity();
	std::vector<int> currentVehicleSolution;
	std::vector<std::vector<double>> DistanceMatrix = pis.getDistanceMatrix();
	std::vector<double> WarehousesDistance = pis.getDistanceWarehouses();
	int current_customer = -1;
	int current_vehicle = -1;
	int current_index = -1;
  int push_vehicle = 0;
  double saving_best = std::numeric_limits<double>::max();
  double saving_current = std::numeric_limits<double>::max();

	for(int i=0; i< customers.size(); i++){
		customers[i] = i;
	}

  while(customers.size() > 0){

    for(int i = 0; i < customers.size(); i++){
      if(currentVehicleSolution.size() != 0){

        for(int j = 0; j < currentVehicleSolution.size() ; j++){
          if( (j == 0) || (j == currentVehicleSolution.size() - 1) ){
            saving_current = WarehousesDistance[i] + DistanceMatrix[i][j] - WarehousesDistance[j];
          }

          else{
            saving_current = DistanceMatrix[j][i] + DistanceMatrix[j+1][i] - DistanceMatrix[j][j+1];
          }

          if(saving_current < saving_best){
            saving_best = saving_current;

            push_vehicle = j;
          }
        }

      }

      else{

        saving_current = 2*(WarehousesDistance[customers[i]]);
        if(saving_current < saving_best){
          saving_best = saving_current;
          push_vehicle = 0;
        }

      }
      current_customer = customers[i];
      current_index = i;
    }

    for(int q = 0; q < customers.size(); q++){
        std::cout << customers[q] << " ";
    }
    std::cout << std::endl;


    if((used + demands[current_customer]) <= vehicle_capacity) {
      currentVehicleSolution.insert(currentVehicleSolution.begin() + push_vehicle, 1, current_customer);
      customers.erase(customers.begin()+current_index);
      used += demands[current_customer];
    }

    else{
      solution.push_back(currentVehicleSolution);
      currentVehicleSolution.clear();
      used_capacities.push_back(used);
      used = 0;
    }
  }
  current_customer = -1;
	current_index = -1;
  saving_best = std::numeric_limits<double>::max();
  saving_current = std::numeric_limits<double>::max();

	VrpSolution* s = new VrpSolution(pis.computeObjectiveFunction(solution), solution, used_capacities);
 	pis.evaluateSolution(*s);

	return s;
}


emili::Solution* emili::vrp::VrpSavingsInitialSolution::generate(){
	std::vector<int> solution_tsp;
	std::vector<std::vector<int>> solution;
	std::vector <int> used_capacities;

	std::pair<int,int> aux_arc;
	bool rep = false;
	std::vector<std::pair<int,int> > arcs;
	std::vector<float> savings;

	int vehicle_capacity = pis.getVehicleCapacity();
	std::vector<int> customers(pis.getNcustomers());
  	std::vector<int> demands = pis.getDemands();
        std::vector<std::vector<double>> DistanceMatrix = pis.getDistanceMatrix();
        std::vector<double> WarehousesDistance = pis.getDistanceWarehouses();

  	for(int i=0; i<customers.size(); i++){
    		customers[i] = i;
  	}




	for(int i = 0; i < customers.size(); i++){
		for(int j = 0; j < customers.size(); j++){

			for(int k = 0; k < arcs.size(); k++){

				if( (arcs[k].first  == i and arcs[k].second == j) || (arcs[k].first  == j and arcs[k].second == i) ){
					rep = true;
				}
			}
			if( (i != j) && rep == false ){
				savings.push_back(WarehousesDistance[i] -  DistanceMatrix[i][j]  + WarehousesDistance[j]);
				aux_arc.first = i;
				aux_arc.second = j;
				arcs.push_back(aux_arc);
			}
			rep = false;
		}
	}

	for(int i = savings.size()-1 ; i >= 0 ; i--){
		if(savings[i] < 0){
			savings.erase(savings.begin() + i);
			arcs.erase(arcs.begin() + i);
		}
	}

	float aux;

	std::pair<int,int> aux2;
	for(int i = 0 ; i <  savings.size() ; i++){
		for(int j = 0 ; j < savings.size(); j++){
			if(savings[i] > savings[j]){
				aux = savings[i];
				savings[i] = savings[j];
				savings[j] = aux;

				aux2.first = arcs[i].first;
				aux2.second = arcs[i].second;

				arcs[i].first = arcs[j].first;
				arcs[i].second = arcs[j].second;

                                arcs[j].first = aux2.first;
                                arcs[j].second = aux2.second;

			}
		}
	}


	bool flag2 = true;
	for(int i = 0; i < savings.size(); i++){
		flag2 = true;
		if( i == 0){
			solution_tsp.push_back(arcs[i].first);
			solution_tsp.push_back(arcs[i].second);
		}
		else{
			if(solution_tsp[solution_tsp.size()-1] == arcs[i].first){
				for(int j = 0; j < solution_tsp.size();j++){
					if(solution_tsp[j] == arcs[i].first){
						flag2 = false;
					}
				}
				if(flag2 == true){
					solution_tsp.push_back(arcs[i].first);
				}
			}
			else{
        for(int j = 0; j < solution_tsp.size();j++){
          if(solution_tsp[j] == arcs[i].second){
            flag2 = false;
          }
        }
        if(flag2 == true){
          solution_tsp.push_back(arcs[i].second);
        }
			}
		}
	}



	/*for(int i = 0 ; i < solution_tsp.size(); i++){
          std::cout << solution_tsp[i]  << std::endl;
  }*/



	int flag = 0;
	


	for(int i = 0; i <customers.size() ; i++){
		//std::cout<< " el customer es " <<customers[i] << std::endl;
		for(int j = 0; j < solution_tsp.size(); j++){
			//std::cout<< " en la ruta es  " <<solution_tsp[j] << std::endl;
			if(solution_tsp[j] == customers[i]){
				flag = 1;
			}
		}
		if(flag == 0){
			std::cout << "entrando" <<std::endl;
			solution_tsp.push_back(customers[i]);
		}
		flag = 0;
	}

	std::vector<int> vehic;
	float demand_vehic = 0;
	for(int i = 0; i < solution_tsp.size(); i++){
		if(demand_vehic + demands [solution_tsp[i]] > vehicle_capacity){
			solution.push_back(vehic);
			vehic.clear();
			used_capacities.push_back(demand_vehic);
			demand_vehic = 0;
		}
		vehic.push_back(solution_tsp[i]);
		demand_vehic += demands [solution_tsp[i]];

		if(solution_tsp.size()-1 == i && vehic.size() != 0){
			solution.push_back(vehic);
      used_capacities.push_back(demand_vehic);
		}

	}
	for(int i = 0 ;i < solution.size(); i++){
		for(int j = 0 ;j < solution[i].size(); j++){
			std::cout << solution[i][j] << " ";
		}
		std::cout << std::endl;
	}

	 std::cout <<pis.computeObjectiveFunction(solution)<<std::endl;
	VrpSolution* s = new VrpSolution(pis.computeObjectiveFunction(solution), solution, used_capacities);

	pis.evaluateSolution(*s);

	return  s;

}

emili::Solution* emili::vrp::VrpSavingsParam1InitialSolution::generate(){
	std::vector<int> solution_tsp;
	std::vector<std::vector<int>> solution;
	std::vector <int> used_capacities;

	std::pair<int,int> aux_arc;
	bool rep = false;
	std::vector<std::pair<int,int> > arcs;
	std::vector<float> savings;

	int vehicle_capacity = pis.getVehicleCapacity();
	std::vector<int> customers(pis.getNcustomers());
  	std::vector<int> demands = pis.getDemands();
        std::vector<std::vector<double>> DistanceMatrix = pis.getDistanceMatrix();
        std::vector<double> WarehousesDistance = pis.getDistanceWarehouses();

  	for(int i=0; i<customers.size(); i++){
    		customers[i] = i;
  	}




	for(int i = 0; i < customers.size(); i++){
		for(int j = 0; j < customers.size(); j++){

			for(int k = 0; k < arcs.size(); k++){

				if( (arcs[k].first  == i and arcs[k].second == j) || (arcs[k].first  == j and arcs[k].second == i) ){
					rep = true;
				}
			}
			if( (i != j) && rep == false ){
				savings.push_back(WarehousesDistance[i] -  DistanceMatrix[i][j]  + WarehousesDistance[j] - DistanceMatrix[i][j] * (( (emili::generateRealRandomNumber()) * 2) +0.5));
				aux_arc.first = i;
				aux_arc.second = j;
				arcs.push_back(aux_arc);
			}
			rep = false;
		}
	}

	//std::cout <<"error 1"<<std::endl;
	for(int i = savings.size()-1 ; i >= 0 ; i--){
		if(savings[i] < 0){
			savings.erase(savings.begin() + i);
			arcs.erase(arcs.begin() + i);
		}
	}

	//std::cout << std::endl;

        /*for(int i = 0 ; i < arcs.size(); i++){
                std::cout << arcs[i].first << " " << arcs[i].second << std::endl;
        }*/


	//std::cout << savings.size()<< std::endl;
	/*for(int i = 0 ; i < savings.size(); i++){
		std::cout << savings[i]  << std::endl;
	}*/

	//std::cout <<"error 2"<<std::endl;
	float aux;
//std::cout << savings.size() << std::endl;
//std::cout << arcs.size() << std::endl;	
	std::pair<int,int> aux2;
	for(int i = 0 ; i <  savings.size() ; i++){
		for(int j = 0 ; j < savings.size(); j++){
			if(savings[i] > savings[j]){
				aux = savings[i];
				savings[i] = savings[j];
				savings[j] = aux;

				aux2.first = arcs[i].first;
				aux2.second = arcs[i].second;

				arcs[i].first = arcs[j].first;
				arcs[i].second = arcs[j].second;

                                arcs[j].first = aux2.first;
                                arcs[j].second = aux2.second;

			}
		}
	}


 	//std::cout <<"error 3"<<std::endl;
	bool flag2 = true;
	for(int i = 0; i < savings.size(); i++){
		flag2 = true;
		if( i == 0){
			solution_tsp.push_back(arcs[i].first);
			solution_tsp.push_back(arcs[i].second);
		}
		else{
			if(solution_tsp[solution_tsp.size()-1] == arcs[i].first){
				for(int j = 0; j < solution_tsp.size();j++){
					if(solution_tsp[j] == arcs[i].first){
						flag2 = false;
					}
				}
				if(flag2 == true){
					solution_tsp.push_back(arcs[i].first);
				}
				//solution_tsp.push_back(arcs[i].first);
			}
			else{
                                for(int j = 0; j < solution_tsp.size();j++){
                                        if(solution_tsp[j] == arcs[i].second){
                                                flag2 = false;
                                        }
                                }
                                if(flag2 == true){
                                        solution_tsp.push_back(arcs[i].second);
                                }


				//solution_tsp.push_back(arcs[i].second);
			}
		}
	}

	//std::cout << "empezando" << std::endl;


	for(int i = 0 ; i < solution_tsp.size(); i++){
                std::cout << solution_tsp[i]  << std::endl;
        }
	//std::cout <<"final"<<std::endl;



	int flag = 0;
	
	/*for(int i = 0; i < customers.size() ; i++){
		std::cout<< " el customer ess " <<customers[i] << std::endl;
	}*/


	for(int i = 0; i <customers.size() ; i++){
		//std::cout<< " el customer es " <<customers[i] << std::endl;
		for(int j = 0; j < solution_tsp.size(); j++){
			//std::cout<< " en la ruta es  " <<solution_tsp[j] << std::endl;
			if(solution_tsp[j] == customers[i]){
				flag = 1;
			}
		}
		if(flag == 0){
			std::cout << "entrando" <<std::endl;
			solution_tsp.push_back(customers[i]);
		}
		flag = 0;
	}
	/*std::cout <<"error 5"<<std::endl;
	for(int i = 0 ; i < solution_tsp.size(); i++){
                std::cout << solution_tsp[i]  << std::endl;
        }*/

	std::vector<int> vehic;
	float demand_vehic = 0;
	for(int i = 0; i < solution_tsp.size(); i++){
		//std::cout <<"vehiculo actual "<< solution_tsp[i]<< std::endl;
		if(demand_vehic + demands [solution_tsp[i]] > vehicle_capacity){
			solution.push_back(vehic);
			vehic.clear();
			used_capacities.push_back(demand_vehic);
			demand_vehic = 0;
		}
		vehic.push_back(solution_tsp[i]);
		demand_vehic += demands [solution_tsp[i]];
		/*if(demand_vehic + demands [solution_tsp[i]] <= vehicle_capacity){
			vehic.push_back(solution_tsp[i]);
			demand_vehic += demands [solution_tsp[i]];
			if( solution_tsp.size()-1 == i){
				solution.push_back(vehic);
                        	used_capacities.push_back(demand_vehic);
			}
		}
		if(demand_vehic >= vehicle_capacity){
			solution.push_back(vehic);
			used_capacities.push_back(demand_vehic);
			vehic.clear();
			demand_vehic = 0;
		}*/

		if(solution_tsp.size()-1 == i && vehic.size() != 0){
			solution.push_back(vehic);
                        used_capacities.push_back(demand_vehic);
		}

	}
	//std::cout <<"error 6"<<std::endl;
	for(int i = 0 ;i < solution.size(); i++){
		for(int j = 0 ;j < solution[i].size(); j++){
			std::cout << solution[i][j] << " ";
		}
		std::cout << std::endl;
	}
	//std::cout << solution.size() << std::endl;
	//std::cout << "capacidad" << std::endl;


	//std::cout <<"holi"<<std::endl;
	 std::cout <<pis.computeObjectiveFunction(solution)<<std::endl;
	VrpSolution* s = new VrpSolution(pis.computeObjectiveFunction(solution), solution, used_capacities);

	pis.evaluateSolution(*s);

	return  s;

}



emili::Solution* emili::vrp::VrpSavingsParam2InitialSolution::generate(){
	std::vector<int> solution_tsp;
	std::vector<std::vector<int>> solution;
	std::vector <int> used_capacities;

	std::pair<int,int> aux_arc;
	bool rep = false;
	std::vector<std::pair<int,int> > arcs;
	std::vector<float> savings;

	int vehicle_capacity = pis.getVehicleCapacity();
	std::vector<int> customers(pis.getNcustomers());
  	std::vector<int> demands = pis.getDemands();
        std::vector<std::vector<double>> DistanceMatrix = pis.getDistanceMatrix();
        std::vector<double> WarehousesDistance = pis.getDistanceWarehouses();
        double sum_ = 0;
        double mean_demand = 0;
  	for(int i=0; i<customers.size(); i++){
    		customers[i] = i;
  	}


  	for(int i = 0; i < demands.size(); i++){
  		sum_ = sum_+demands[i];
  	}
  	mean_demand = sum_ / demands.size();
	for(int i = 0; i < customers.size(); i++){
		for(int j = 0; j < customers.size(); j++){

			for(int k = 0; k < arcs.size(); k++){

				if( (arcs[k].first  == i and arcs[k].second == j) || (arcs[k].first  == j and arcs[k].second == i) ){
					rep = true;
				}
			}
			if( (i != j) && rep == false ){
				savings.push_back(WarehousesDistance[i] -  DistanceMatrix[i][j]  + WarehousesDistance[j] - DistanceMatrix[i][j]*lambda_ + abs(WarehousesDistance[i] - WarehousesDistance[j]) + v_ * ((demands[i] + demands[j])/mean_demand) );
				aux_arc.first = i;
				aux_arc.second = j;
				arcs.push_back(aux_arc);
			}
			rep = false;
		}
	}

	//std::cout <<"error 1"<<std::endl;
	for(int i = savings.size()-1 ; i >= 0 ; i--){
		if(savings[i] < 0){
			savings.erase(savings.begin() + i);
			arcs.erase(arcs.begin() + i);
		}
	}

	//std::cout << std::endl;

        /*for(int i = 0 ; i < arcs.size(); i++){
                std::cout << arcs[i].first << " " << arcs[i].second << std::endl;
        }*/


	//std::cout << savings.size()<< std::endl;
	/*for(int i = 0 ; i < savings.size(); i++){
		std::cout << savings[i]  << std::endl;
	}*/

	//std::cout <<"error 2"<<std::endl;
	float aux;
//std::cout << savings.size() << std::endl;
//std::cout << arcs.size() << std::endl;	
	std::pair<int,int> aux2;
	for(int i = 0 ; i <  savings.size() ; i++){
		for(int j = 0 ; j < savings.size(); j++){
			if(savings[i] > savings[j]){
				aux = savings[i];
				savings[i] = savings[j];
				savings[j] = aux;

				aux2.first = arcs[i].first;
				aux2.second = arcs[i].second;

				arcs[i].first = arcs[j].first;
				arcs[i].second = arcs[j].second;

                                arcs[j].first = aux2.first;
                                arcs[j].second = aux2.second;

			}
		}
	}


 	//std::cout <<"error 3"<<std::endl;
	bool flag2 = true;
	for(int i = 0; i < savings.size(); i++){
		flag2 = true;
		if( i == 0){
			solution_tsp.push_back(arcs[i].first);
			solution_tsp.push_back(arcs[i].second);
		}
		else{
			if(solution_tsp[solution_tsp.size()-1] == arcs[i].first){
				for(int j = 0; j < solution_tsp.size();j++){
					if(solution_tsp[j] == arcs[i].first){
						flag2 = false;
					}
				}
				if(flag2 == true){
					solution_tsp.push_back(arcs[i].first);
				}
				//solution_tsp.push_back(arcs[i].first);
			}
			else{
                                for(int j = 0; j < solution_tsp.size();j++){
                                        if(solution_tsp[j] == arcs[i].second){
                                                flag2 = false;
                                        }
                                }
                                if(flag2 == true){
                                        solution_tsp.push_back(arcs[i].second);
                                }


				//solution_tsp.push_back(arcs[i].second);
			}
		}
	}



	for(int i = 0 ; i < solution_tsp.size(); i++){
                std::cout << solution_tsp[i]  << std::endl;
        }



	int flag = 0;
	
	/*for(int i = 0; i < customers.size() ; i++){
		std::cout<< " el customer ess " <<customers[i] << std::endl;
	}*/


	for(int i = 0; i <customers.size() ; i++){
		//std::cout<< " el customer es " <<customers[i] << std::endl;
		for(int j = 0; j < solution_tsp.size(); j++){
			//std::cout<< " en la ruta es  " <<solution_tsp[j] << std::endl;
			if(solution_tsp[j] == customers[i]){
				flag = 1;
			}
		}
		if(flag == 0){
			std::cout << "entrando" <<std::endl;
			solution_tsp.push_back(customers[i]);
		}
		flag = 0;
	}
	/*std::cout <<"error 5"<<std::endl;
	for(int i = 0 ; i < solution_tsp.size(); i++){
                std::cout << solution_tsp[i]  << std::endl;
        }*/

	std::vector<int> vehic;
	float demand_vehic = 0;
	for(int i = 0; i < solution_tsp.size(); i++){
		//std::cout <<"vehiculo actual "<< solution_tsp[i]<< std::endl;
		if(demand_vehic + demands [solution_tsp[i]] > vehicle_capacity){
			solution.push_back(vehic);
			vehic.clear();
			used_capacities.push_back(demand_vehic);
			demand_vehic = 0;
		}
		vehic.push_back(solution_tsp[i]);
		demand_vehic += demands [solution_tsp[i]];
		/*if(demand_vehic + demands [solution_tsp[i]] <= vehicle_capacity){
			vehic.push_back(solution_tsp[i]);
			demand_vehic += demands [solution_tsp[i]];
			if( solution_tsp.size()-1 == i){
				solution.push_back(vehic);
                        	used_capacities.push_back(demand_vehic);
			}
		}
		if(demand_vehic >= vehicle_capacity){
			solution.push_back(vehic);
			used_capacities.push_back(demand_vehic);
			vehic.clear();
			demand_vehic = 0;
		}*/

		if(solution_tsp.size()-1 == i && vehic.size() != 0){
			solution.push_back(vehic);
                        used_capacities.push_back(demand_vehic);
		}

	}
	//std::cout <<"error 6"<<std::endl;
	for(int i = 0 ;i < solution.size(); i++){
		for(int j = 0 ;j < solution[i].size(); j++){
			std::cout << solution[i][j] << " ";
		}
		std::cout << std::endl;
	}
	//std::cout << solution.size() << std::endl;
	//std::cout << "capacidad" << std::endl;


	//std::cout <<"holi"<<std::endl;
	 std::cout <<pis.computeObjectiveFunction(solution)<<std::endl;
	VrpSolution* s = new VrpSolution(pis.computeObjectiveFunction(solution), solution, used_capacities);

	pis.evaluateSolution(*s);

	return  s;

}

emili::Solution* emili::vrp::ChristofidesInitialSolution::generate(){
        std::vector<int> customers(pis.getNcustomers());
	std::vector<int> demands = pis.getDemands();
        std::vector<std::vector<double>> DistanceMatrix = pis.getDistanceMatrix();
        std::vector<double> WarehousesDistance = pis.getDistanceWarehouses();
	std::vector <int> used_capacities;

        for(int i=0; i<customers.size(); i++){
        	customers[i] = i;
        }
	int vehicle_capacity = pis.getVehicleCapacity();
	std::vector<int> adjlist(pis.getNcustomers());   // es necesariamente int?

	// inicio algoritmo de prim 
	std::vector<int> key(pis.getNcustomers());
	std::vector<bool> included(pis.getNcustomers());
	std::vector<int> parent(pis.getNcustomers());
	std::vector<std::vector<int>> solution;

	for (int i = 0; i < pis.getNcustomers(); i++) {
        	key[i] = std::numeric_limits<int>::max();
                included[i] = false;
  	}

	key[0] = 0;
  	parent[0] = -1;

	for (int i = 0; i < pis.getNcustomers() - 1; i++) {
		int k = getMinIndex(key, included);
		included[k] = true;
		for (int j = 0; j < pis.getNcustomers(); j++) {
			if (DistanceMatrix[k][j] && included[j] == false && DistanceMatrix[k][j] < key[j]) {
				parent[j] = k;
				key[j] = DistanceMatrix[k][j];
			}
		}
	}
	for (int i = 0; i < pis.getNcustomers(); i++) {
		int j = parent[i];
		if (j != -1){
			//adjlist[i].push_back(j);
      			//adjlist[j].push_back(i);
			adjlist[i] = i;
			adjlist[j] = j;
		}
	}
	/*for (auto i: adjlist)
    		std::cout << i << ' ';*/


	// final algoritmo de prim
	
	// inicio perfect matching 
		std::vector<int>odds = findOdds(adjlist);
		int closest, length;
		std::vector<int>::iterator tmp, first;
		
		while (!odds.empty()) {
			std::cout <<"ERROR CRIS 3"<< std::endl;
			first = odds.begin();
			std::vector<int>::iterator it = odds.begin() + 1;
    			std::vector<int>::iterator end = odds.end();
    			length = std::numeric_limits<int>::max();
			for (; it != end; ++it) {
				if (DistanceMatrix[*first][*it] < length) {
					length = DistanceMatrix[*first][*it];
					closest = *it;
        				tmp = it;
				}
			}
			//std::cout <<"ERROR CRIS 4"<< std::endl;
    			//adjlist[*first].push_back(closest);
    			//adjlist[closest].push_back(*first);
			adjlist[*first] = closest;
			//std::cout <<"ERROR CRIS 5"<< std::endl;
			adjlist[closest] = *first;
			//std::cout <<"ERROR CRIS 6"<< std::endl;
			std::cout <<odds.size()<< std::endl;
			if(odds.size() == 1){
				odds.erase(first);
			}
			else{
				odds.erase(tmp);
                        	odds.erase(first);
			}
    			/*odds.erase(tmp);
			std::cout <<"ERROR CRIS 7"<< std::endl;
    			odds.erase(first);*/
		}
		//std::cout <<"ERROR CRIS 4"<< std::endl;
	// final perfect matching
	//std::vector<vector<int>> path(pis.getNcustomers(), pis.getNcustomers());
	int best = INT_MAX;
	int bestIndex;
	std::vector<std::vector<int>> matrix_n_n(pis.getNcustomers(), std::vector<int>(pis.getNcustomers())   );
	for (long t = 0; t < pis.getNcustomers(); t++) {
		int result = findBestPath(t,adjlist);
		matrix_n_n[t][0] = t;
		matrix_n_n[t][1] = result;
		if ( matrix_n_n[t][1] < best) {
			bestIndex = matrix_n_n[t][0];
			best = matrix_n_n[t][1];
		}
	}
	////
	int pathLength;
	std::vector<int> circuit;

	euler_tour(bestIndex,circuit,adjlist);
	make_hamiltonian(circuit,pathLength);
	////

	//circuit es el tsp
	int flag = 0;
	for(int i = 0; i <customers.size() ; i++){
		for(int j = 0; j < circuit.size(); j++){
			if(circuit[j] == customers[i]){
				flag = 1;
			}
		}
		if(flag == 0){
			circuit.push_back(customers[i]);
		}
		flag = 0;
	}

	std::vector<int> vehic;
	float demand_vehic = 0;
	for(int i = 0; i < circuit.size(); i++){
		if(demand_vehic + demands [circuit[i]] > vehicle_capacity){
			solution.push_back(vehic);
			vehic.clear();
			used_capacities.push_back(demand_vehic);
			demand_vehic = 0;
		}
		vehic.push_back(circuit[i]);
		demand_vehic += demands [circuit[i]];

		if(circuit.size()-1 == i && vehic.size() != 0){
			solution.push_back(vehic);
                        used_capacities.push_back(demand_vehic);
		}

	}
	VrpSolution* s = new VrpSolution(pis.computeObjectiveFunction(solution), solution, used_capacities);

	pis.evaluateSolution(*s);

	return  s;

}

int emili::vrp::ChristofidesInitialSolution::getMinIndex(std::vector<int> key, std::vector<bool> included){
	int min = std::numeric_limits<int>::max();
	int min_index;
	for (int i = 0; i < pis.getNcustomers(); i++){
		if(included[i] == false && key[i] < min) {
			min = key[i];
      			min_index = i;
		}
	}
	return min_index;
}

std::vector<int> emili::vrp::ChristofidesInitialSolution::findOdds(std::vector<int> adjlist){
	std::vector<int> odds;
	for(int i = 0; i < pis.getNcustomers(); i++){
        	if((adjlist[i] % 2) != 0) {
                	odds.push_back(i);
                }
	}
	return odds;
}


int emili::vrp::ChristofidesInitialSolution::findBestPath(int t,std::vector<int> adjlist){
	std::vector<int> path;
	euler_tour(t, path,adjlist);
	int length;
	make_hamiltonian(path, length);
	return length;
}

void emili::vrp::ChristofidesInitialSolution::euler_tour(int start,std::vector<int> &path, std::vector<int> adjlist){
	std::vector<int> *tempList = new std::vector<int>[pis.getNcustomers()];
	//std::vector<int> tempList(pis.getNcustomers());
	for(int i = 0; i < pis.getNcustomers(); i++){
		tempList[i].resize(adjlist.size());
		//tempList[i] = adjlist[i];
	}
	std::stack<int> stack;
	int pos = start;
	path.push_back(start);
	while(!stack.empty() || tempList[pos].size() > 0){
		if(tempList[pos].empty()){
			path.push_back(pos);
			pos = stack.top();
			stack.pop();
		}
		else{
			stack.push(pos);
			int neighbor = tempList[pos].back();
			tempList[pos].pop_back();
			for(int i = 0; i < tempList[neighbor].size(); i++){
				if(tempList[neighbor][i] == pos){
					tempList[neighbor].erase(tempList[neighbor].begin()+i);
				}
			}
			pos = neighbor;
		}
	}
	path.push_back(pos);
}

void emili::vrp::ChristofidesInitialSolution::make_hamiltonian(std::vector<int> &path, int &pathCost){
	bool *visited = new bool[pis.getNcustomers()];
	std::vector<std::vector<double>> DistanceMatrix = pis.getDistanceMatrix();
	for(int i = 0; i < pis.getNcustomers(); i++){
		visited[i] = 0;
	}
	
	pathCost = 0;

	int root = path.front();
	std::vector<int>::iterator cur = path.begin();
	std::vector<int>::iterator iter = path.begin()+1;
	visited[root] = 1;

	bool addMore = true;
	while(iter != path.end()){
		if(!visited[*iter]){
			pathCost += DistanceMatrix[*cur][*iter];
			cur = iter;
			visited[*cur] = 1;
			iter = cur + 1;
		}
		else{
			iter = path.erase(iter);
		}
	}
	if ( iter != path.end() ){
		pathCost += DistanceMatrix[*cur][*iter];
	}

}


emili::Solution* emili::vrp::LocalSearchInitial::generate(){
  VrpSolution* s = ( (emili::vrp::VrpSolution*) lls->search());

	return s;
}

/*----------------------------------------------------------------------------------------------------------------*
 *-------------------------------------------------SOLUTION-------------------------------------------------------*
 *----------------------------------------------------------------------------------------------------------------*/

emili::Solution* emili::vrp::VrpSolution::clone()
{

  emili::Solution* new_solution = new emili::vrp::VrpSolution(this->solution_value, this->representation, this->used_capacity);
  new_solution->termcriterion = this->termcriterion;
  return new_solution;
  //return new emili::vrp::VrpSolution(this->solution_value, this->representation, this->used_capacity);
}

bool emili::vrp::VrpSolution::operator==(Solution& a) 
{
  bool ret = this->solution_value == a.getSolutionValue();
  if (ret) {
    emili::vrp::VrpSolution* ap = (emili::vrp::VrpSolution*) &a;
    std::vector < std::vector<int>>& ap_trucks = ap->getSolution();
    if (ap_trucks.size() != this->representation.size()) return false;
    for (int t = 0; t<ap_trucks.size(); t++) {
      if (ap_trucks[t].size() != this->representation[t].size()) return false;
      for (int c=0; c<ap_trucks[t].size(); c++) {
        if (ap_trucks[t][c] != this->representation[t][c]) return false;
      }
    }
  }
  return ret;
}

const void* emili::vrp::VrpSolution::getRawData()const
{
  return &data;
}
 
void emili::vrp::VrpSolution::setRawData(const void* data)
{ 
  emili::vrp::vrpData* vdata = (emili::vrp::vrpData*) data;
  std::vector< std::vector<int>> r = vdata->routes;
  std::vector<int> u = vdata->used_capacity;
  
  this->data.routes = r;
  this->data.used_capacity = u;
  
  this->representation = r;
  this->used_capacity = u;
  this->last_backup = false;
}

std::string emili::vrp::VrpSolution::getSolutionRepresentation()
{
  std::ostringstream oss;
  int size = representation.size();
  oss << "{";
  for(int i=0; i<size; i++){
    for(int j=0;j<representation[i].size();j++){
      oss << representation[i][j];
      if (j!=(representation[i].size()-1)) oss << ",";
    }
    oss << ";";
  }
  oss << "}";
  return oss.str();
}

void emili::vrp::VrpSolution::printSolutionRepresentation()
{
  int size = representation.size();
  std::cout<< "{" <<std::endl;

  for(int i=0; i<size; i++){
    for(int j=0;j<representation[i].size();j++){
      std::cout << representation[i][j]+1 << ", "; 
    }
    std::cout << std::endl;
  }
  std::cout << std::endl << "} ";
  
  double f = this->solution_value;
  std::cout << f << std::endl;
}

void emili::vrp::VrpSolution::printObjetivefunction()
{
  double f = this->solution_value;
  std::cout<< "La funcion objetivo es: " << f << " unidades de distancia" <<std::endl;
}

std::vector< std::vector< int > > & emili::vrp::VrpSolution::getSolution()
{
  return this->representation;
}

void emili::vrp::VrpSolution::setSolution(std::vector< std::vector< int > > & sol)
{
  this->last_representation = this->representation;
  this->last_backup = true;
  this->representation = sol;
  this->data.routes= representation;
}

double emili::vrp::VrpSolution::getSolutionValue()
{
  return this->solution_value;
}

void emili::vrp::VrpSolution::setSolutionValue(double solutionValue)
{
  this->last_solution_value = this->solution_value;
  this->solution_value = solutionValue;
}

bool emili::vrp::VrpSolution::isFeasible()
{
  int n_customers = 0;
  std::vector< int > visited;
  for(int i=0; i<representation.size(); i++){
    for(int j=0;j<representation[i].size();j++){
      n_customers++;
      visited.push_back(0);
    }
  }

  int aux = 0;
  for(int i=0; i<representation.size(); i++) {
    for(int j=0;j<representation[i].size();j++) {
      aux = representation[i][j];
      if (aux > visited.size()) return false;
      visited[aux]++;
      if (visited[aux]>1) return false;
    }
  }
  return true;
}

int emili::vrp::VrpSolution::getNVehicles() 
{
  return(representation.size());
}

void emili::vrp::VrpSolution::removeEmptyVehicles () 
{
  std::vector <std::vector <int> > new_representation;
  std::vector <int> new_capacities;
  
  int n_vehicle=0;
  for (int v=0; v<getNVehicles(); v++) {
    if (representation[v].size()!=0) {
      new_representation.push_back(representation[v]);
      new_capacities.push_back(used_capacity[v]);
      n_vehicle++;
    }
  }
  representation = new_representation;
  used_capacity = new_capacities;
  
  data.routes = representation;
  data.used_capacity = used_capacity;
}

std::vector<int> emili::vrp::VrpSolution::getUsedCapacity() 
{
  return(used_capacity);
}

void emili::vrp::VrpSolution::setUsedCapacity(std::vector<int> capacity) 
{
  this->last_used_capacity = this->used_capacity;
  this->used_capacity = capacity;
  this->data.used_capacity = capacity;
}

void emili::vrp::VrpSolution::reverse()
{
  if (last_backup){
    representation = last_representation;
    used_capacity = last_used_capacity;
    //This is reset outside this function by emili
    //solution_value = last_solution_value;
    last_backup = false;
    
    data.routes = representation;
    data.used_capacity = used_capacity;
  }
}

/*----------------------------------------------------------------------------------------------------------------*
 *--------------------------------------------------NEIGHBORHOOD--------------------------------------------------*
 *----------------------------------------------------------------------------------------------------------------*/

/*-------------------------------------------------INTER-ROUTE----------------------------------------------------*/

emili::Neighborhood::NeighborhoodIterator emili::vrp::InterRoute_Neighborhood::begin(emili::Solution* base)
{
  emili::vrp::VrpSolution* solution = ((emili::vrp::VrpSolution*) base);

  calculateNeighborhoodSize(base);
  n_iterations = 1;
  c_iterations = 1;
  last_n_iterations = 1;
  last_c_iterations = 1;
  last_n_size = n_size;
  isFirstTime = true;
  return emili::Neighborhood::NeighborhoodIterator(this,base);
}

void emili::vrp::InterRoute_Neighborhood::reset()
{
  //start position
  next_move = std::make_pair(std::make_pair(0,0),std::make_pair(0,0));
  last_move = std::make_pair(std::make_pair(0,0),std::make_pair(0,0));
  pre_post_customers = std::make_pair(std::make_pair(-1,-1),std::make_pair(-1,-1));
  
  n_iterations = 1;
  c_iterations = 1;
  last_n_iterations = 1;
  last_c_iterations = 1;
  isFirstTime = true;
}

emili::Solution* emili::vrp::InterRoute_Neighborhood::computeStep(emili::Solution* step)  
{
  emili::iteration_increment();
  
  double solution_value = ((emili::vrp::VrpSolution*) step)->getSolutionValue();
  //std::cout << "current solution " << step->getSolutionRepresentation() << " " << step->getSolutionValue() << std::endl;
  

  while(n_iterations <= n_size) {
    setNextNeighborPos(step);
    if (isNextFeasible(step) ){
      // Modify actual Solution for the new one
      makeMove(step);
      //Update neighborhood
      last_n_iterations = n_iterations;
      last_c_iterations = c_iterations;
      last_n_size = n_size;
      calculateNeighborhoodSize(step);
      n_iterations = 1;
      c_iterations = 1;
      last_move = next_move;
      //std::cout << "feasible" << " " << step->getSolutionRepresentation() << step->getSolutionValue() << std::endl;

      return step;
    }  //else std::cout << "infeasible" << " keep " << step->getSolutionRepresentation() << step->getSolutionValue() << std::endl;
  }
  
  return nullptr;
}

void emili::vrp::InterRoute_Neighborhood::reverseLastMove(Solution* step)
{
  ((emili::vrp::VrpSolution*) step)->reverse();
  next_move = last_move;
  n_iterations = last_n_iterations;
  c_iterations = last_c_iterations;
  n_size = last_n_size;
}

/*******************************Shift_1_0_Neighborhood*********************************/

void emili::vrp::Shift_1_0_Neighborhood::calculateNeighborhoodSize(emili::Solution* base) 
{ 
  //Shift neighborhood considers all possible positions to move
  //moves must filtered based on the move
  int n_vehicles = ((emili::vrp::VrpSolution*) base)->getNVehicles();
  n_size = this->n_customers*(this->n_customers + n_vehicles + 1); 
  //n_customers + n_vehicles + 1(new vehicle) is the total number of positions possible for shifting one client
};


bool emili::vrp::Shift_1_0_Neighborhood::isNextFeasible(emili::Solution* base)
{
  std::vector<int> used_capacities = ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int new_used_capacity = 0;
  std::vector<int> demands = pis.getDemands();
  
  int orig_vehicle = next_move.first.first;
  int current_customer = solution[orig_vehicle][next_move.first.second];
  int dest_vehicle = next_move.second.first;
  
  if (dest_vehicle == solution.size()) {
    new_used_capacity = demands[current_customer];
  } else {
    new_used_capacity = used_capacities[dest_vehicle] + demands[current_customer];
  }
  
  if(new_used_capacity > pis.getVehicleCapacity()) {
    return false;
  }
  
  if (solution[orig_vehicle].size() == 1 && dest_vehicle == solution.size()) 
    return false;
  return true;
}

emili::Solution* emili::vrp::Shift_1_0_Neighborhood::random(Solution* currentSolution) 
{
  int orig_vehicle;
  int orig_index;
  int dest_vehicle;
  int dest_index;
  
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();

  do {
    //Get random customer indexes
    //std::cout <<"pegado"<< std::endl;
    orig_vehicle = emili::generateRandomNumber() % solution.size();
    orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
    do {
      dest_vehicle = emili::generateRandomNumber() %  solution.size();
    } while (dest_vehicle==orig_vehicle);
    dest_index = emili::generateRandomNumber() % (solution[dest_vehicle].size()+1);
    next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    //std::cout << "random shift10 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << std::endl; 
  } while (!isNextFeasible(new_solution));
  setAuxiliaryVariables(solution);
  makeMove(new_solution);
  //std::cout <<new_solution->getSolutionValue() << std::endl;
 
  return new_solution;
}

void emili::vrp::Shift_1_0_Neighborhood::setNextNeighborPos(emili::Solution* base)
{
  //std::cout << "prueba " << base->printSolutionRepresentation()  <<std::endl;
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
  do {
    if (!isFirstTime) {
      //n_customers + n_vehicles + 1(new vehicle) is the total number of positions possible for shifting a client
      if (c_iterations > (n_customers + solution.size()+1)) {
        orig_index++;
        if (orig_index >= solution[orig_vehicle].size()){
          orig_index = 0;
          orig_vehicle++;
          if (orig_vehicle >= solution.size())
            orig_vehicle = 0;
        }
        c_iterations = 1;
      } else {
        dest_index++;
        if (dest_vehicle >= solution.size()) {
          //we assume one more vehicle here we check if
          //we are in this virtual vehicle
          dest_vehicle = 0;
          dest_index = 0;
        } else if (dest_index > solution[dest_vehicle].size()) {
          //rolling of the neighbors
          dest_vehicle++;
          dest_index = 0;
        }
      }
    } else {
      isFirstTime = false;
      if (orig_index >= solution[orig_vehicle].size()){
        orig_index = 0;
        orig_vehicle++;
      } 
      if (orig_vehicle >= solution.size()){
        orig_vehicle = 0;
        orig_index = 0;
      }
      if (dest_vehicle >= solution.size()) {
        dest_vehicle = 0;
        dest_index = 0;
      }
      
      if (dest_index > solution[dest_vehicle].size()){
        dest_index =0;
        dest_vehicle++;
      }
    }
    current_customer = solution[orig_vehicle][orig_index];
    //std::cout << "shift10 c:" << current_customer << " o:" << orig_vehicle << "," << orig_index << " d:" << 
      //dest_vehicle << "," << dest_index << " step: " << c_iterations << "/" <<n_customers + solution.size()+1<< "  " << n_iterations << "/" << n_size << std::endl;
     
    next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    setAuxiliaryVariables(solution);
    n_iterations++;
    c_iterations++;
    //std::cout << "prueba " << next_move <<std:endl;
  } while (dest_vehicle==orig_vehicle && n_iterations <= n_size);
  
}

void emili::vrp::Shift_1_0_Neighborhood::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();
  bool empty_vehicle = false;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int current_customer = solution[orig_vehicle][orig_index];
  // remove target customer
  solution[orig_vehicle].erase(solution[orig_vehicle].begin() + orig_index);
  used_capacity[orig_vehicle] = used_capacity[orig_vehicle] - demands[current_customer];
  if (solution[orig_vehicle].size() < 1) empty_vehicle = true;
  // insert target customer
  //std::cout << "empezo" <<std::endl;
  if (dest_vehicle == solution.size()) {

    std::vector <int> n;
    n.push_back(current_customer);
    solution.push_back(n);
    used_capacity.push_back(demands[current_customer]);
  } else {

    solution[dest_vehicle].begin() + dest_index;
    if(solution[dest_vehicle].size() < dest_index){

    	solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index -1), current_customer);
        used_capacity[dest_vehicle] = used_capacity[dest_vehicle] + demands[current_customer];
    }
    else{
    	solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index), current_customer);
    	used_capacity[dest_vehicle] = used_capacity[dest_vehicle] + demands[current_customer];
    }
  } 
  
  
  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
  //std::cout << "fallo 4" << std::endl;
  if (empty_vehicle) {
    //std::cout << "fallo 5" << std::endl;
    ((emili::vrp::VrpSolution*) base )->removeEmptyVehicles();
    //std::cout << "fallo 5" << std::endl;
  } 
  //std::cout << "termino" <<std::endl;
}

double emili::vrp::Shift_1_0_Neighborhood::calculateDeltaSolutionValue(emili::Solution* base)
{
  
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  int current_customer = solution[next_move.first.first][next_move.first.second];
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  int orig_vehicle = next_move.first.first;
  int dest_vehicle = next_move.second.first;
  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  
  if (orig_vehicle == dest_vehicle) return(solution_value);
  
  // remove origin distances
  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[current_customer];
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][current_customer];
  }
  if (orig_post_customer == -1) {
    delta = delta - dis_depot[current_customer];
  } else {
    delta = delta - dis_matrix[current_customer][orig_post_customer];
  }
  //std::cout << "delta " << delta << std::endl;
  
  // add origin distances
  if (orig_prev_customer == -1) {
    //if both are -1 there is nothing to add
    if (orig_post_customer != -1) {
      delta = delta + dis_depot[orig_post_customer];
    } 
  } else {
    if (orig_post_customer == -1) {
      delta = delta + dis_depot[orig_prev_customer];
    } else {
      delta = delta + dis_matrix[orig_prev_customer][orig_post_customer];
    }
  }
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances
  if (dest_prev_customer == -1) {
    //If both are -1 there nothing to remove
    if (dest_post_customer != -1) {
      delta = delta - dis_depot[dest_post_customer];
    }
  } else {
    if (dest_post_customer == -1) {
      delta = delta - dis_depot[dest_prev_customer];
    } else {
      delta = delta - dis_matrix[dest_prev_customer][dest_post_customer];
    }
  }
  //std::cout << "delta " << delta << std::endl;
  
  // add destination distances
  if (dest_prev_customer == -1) {
    if (dest_post_customer == -1) {
      delta = delta + 2*dis_depot[current_customer];
    } else {
      delta = delta + dis_depot[current_customer] + dis_matrix[current_customer][dest_post_customer];
    }
  } else {
    if (dest_post_customer == -1) {
      delta = delta + dis_matrix[dest_prev_customer][current_customer] + dis_depot[current_customer];
    } else {
      delta = delta + dis_matrix[dest_prev_customer][current_customer] + dis_matrix[current_customer][dest_post_customer];
    }
  }

  new_value = solution_value + delta;
  //std::cout << "delta " << delta << " current " << solution_value << " new " << new_value << std::endl;
  
  return new_value;
}

void emili::vrp::Shift_1_0_Neighborhood::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  //Set previous and posterior costuments
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[orig_vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[orig_vehicle].size()-1))
    orig_post_customer = solution[orig_vehicle][orig_index + 1];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[dest_vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_vehicle!=solution.size() && dest_index < solution[dest_vehicle].size())
    dest_post_customer = solution[dest_vehicle][dest_index];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}

/*******************************Shift_1_0_Neighborhood_With_Reparation*********************************/

void emili::vrp::Shift_1_0_Neighborhood_With_Reparation::calculateNeighborhoodSize(emili::Solution* base) 
{ 
  //Shift neighborhood considers all possible positions to move
  //moves must filtered based on the move
  int n_vehicles = ((emili::vrp::VrpSolution*) base)->getNVehicles();
  n_size = this->n_customers*(this->n_customers + n_vehicles + 1); 
  //n_customers + n_vehicles + 1(new vehicle) is the total number of positions possible for shifting one client
};


bool emili::vrp::Shift_1_0_Neighborhood_With_Reparation::isNextFeasible(emili::Solution* base)
{
  /*std::vector<int> used_capacities = ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int new_used_capacity = 0;
  std::vector<int> demands = pis.getDemands();
  
  int orig_vehicle = next_move.first.first;
  int current_customer = solution[orig_vehicle][next_move.first.second];
  int dest_vehicle = next_move.second.first;
  
  if (dest_vehicle == solution.size()) {
    new_used_capacity = demands[current_customer];
  } else {
    new_used_capacity = used_capacities[dest_vehicle] + demands[current_customer];
  }
  
  if(new_used_capacity > pis.getVehicleCapacity()) {
    return false;
  }
  
  if (solution[orig_vehicle].size() == 1 && dest_vehicle == solution.size()) 
    return false;*/
  return true;
}

emili::Solution* emili::vrp::Shift_1_0_Neighborhood_With_Reparation::random(Solution* currentSolution) 
{
  int orig_vehicle;
  int orig_index;
  int dest_vehicle;
  int dest_index;
  
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();
  do {
    //Get random customer indexes
    //std::cout <<"pegado"<< std::endl;
    orig_vehicle = emili::generateRandomNumber() % solution.size();
    orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
    do {
      dest_vehicle = emili::generateRandomNumber() %  solution.size();
    } while (dest_vehicle==orig_vehicle);
    dest_index = emili::generateRandomNumber() % (solution[dest_vehicle].size()+1);
    next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    //std::cout << "random shift10 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << std::endl; 
  } while (!isNextFeasible(new_solution));
  setAuxiliaryVariables(solution);
  makeMove(new_solution);
  //std::cout <<new_solution->getSolutionValue() << std::endl;
 
  return new_solution;
}

void emili::vrp::Shift_1_0_Neighborhood_With_Reparation::setNextNeighborPos(emili::Solution* base)
{
  //std::cout << "prueba " << base->printSolutionRepresentation()  <<std::endl;
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
  do {
    if (!isFirstTime) {
      //n_customers + n_vehicles + 1(new vehicle) is the total number of positions possible for shifting a client
      if (c_iterations > (n_customers + solution.size()+1)) {
        orig_index++;
        if (orig_index >= solution[orig_vehicle].size()){
          orig_index = 0;
          orig_vehicle++;
          if (orig_vehicle >= solution.size())
            orig_vehicle = 0;
        }
        c_iterations = 1;
      } else {
        dest_index++;
        if (dest_vehicle >= solution.size()) {
          //we assume one more vehicle here we check if
          //we are in this virtual vehicle
          dest_vehicle = 0;
          dest_index = 0;
        } else if (dest_index > solution[dest_vehicle].size()) {
          //rolling of the neighbors
          dest_vehicle++;
          dest_index = 0;
        }
      }
    } else {
      isFirstTime = false;
      if (orig_index >= solution[orig_vehicle].size()){
        orig_index = 0;
        orig_vehicle++;
      } 
      if (orig_vehicle >= solution.size()){
        orig_vehicle = 0;
        orig_index = 0;
      }
      if (dest_vehicle >= solution.size()) {
        dest_vehicle = 0;
        dest_index = 0;
      }
      
      if (dest_index > solution[dest_vehicle].size()){
        dest_index =0;
        dest_vehicle++;
      }
    }
    current_customer = solution[orig_vehicle][orig_index];
    //std::cout << "shift10 c:" << current_customer << " o:" << orig_vehicle << "," << orig_index << " d:" << 
      //dest_vehicle << "," << dest_index << " step: " << c_iterations << "/" <<n_customers + solution.size()+1<< "  " << n_iterations << "/" << n_size << std::endl;
     
    next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    setAuxiliaryVariables(solution);
    n_iterations++;
    c_iterations++;
    //std::cout << "prueba " << next_move <<std:endl;
  } while (dest_vehicle==orig_vehicle && n_iterations <= n_size);
  
}

void emili::vrp::Shift_1_0_Neighborhood_With_Reparation::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();
  bool empty_vehicle = false;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int current_customer = solution[orig_vehicle][orig_index];
  // remove target customer
  solution[orig_vehicle].erase(solution[orig_vehicle].begin() + orig_index);
  used_capacity[orig_vehicle] = used_capacity[orig_vehicle] - demands[current_customer];
  if (solution[orig_vehicle].size() < 1) empty_vehicle = true;
  // insert target customer
  //std::cout << "empezo" <<std::endl;
  if (dest_vehicle == solution.size()) {

    std::vector <int> n;
    n.push_back(current_customer);
    solution.push_back(n);
    used_capacity.push_back(demands[current_customer]);
  } else {

    solution[dest_vehicle].begin() + dest_index;
    if(solution[dest_vehicle].size() < dest_index){

    	solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index -1), current_customer);
        used_capacity[dest_vehicle] = used_capacity[dest_vehicle] + demands[current_customer];
    }
    else{
    	solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index), current_customer);
    	used_capacity[dest_vehicle] = used_capacity[dest_vehicle] + demands[current_customer];
    }
  } 
  int vehicle_capacity = pis.getVehicleCapacity();
  int random_change = 0;
  std::vector <int> aux;
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();

  ((emili::vrp::VrpSolution*) base )->removeEmptyVehicles();
  for(int i = 0; i < solution.size(); i++){

    while(used_capacity[i] > vehicle_capacity){
    
      if(solution[i].size() == 0){
        continue;
      }
      random_change = emili::generateRandomNumber() % solution[i].size();
      aux.push_back(solution[i][random_change]);
      
      if(random_change == 0){
        solution_value += dis_depot[solution[i][random_change + 1]] - dis_depot[solution[i][random_change]];
        solution_value += - dis_matrix[solution[i][random_change]] [solution[i][random_change + 1]];
      }
      else if(random_change == solution[i].size() - 1){
        solution_value += dis_depot[solution[i][random_change - 1]] - dis_depot[solution[i][random_change]] - dis_matrix[solution[i][random_change]][solution[i][random_change - 1]];
      }
      else{
        solution_value += dis_matrix[solution[i][random_change - 1]][solution[i][random_change + 1]];
        solution_value += - dis_matrix[solution[i][random_change]][solution[i][random_change - 1]];
        solution_value += - dis_matrix[solution[i][random_change]][solution[i][random_change + 1]];
      }
      solution_value += 2*dis_depot[solution[i][random_change]];

      used_capacity[i] -= demands[solution[i][random_change]];
      used_capacity.push_back(demands[solution[i][random_change]]);
      solution[i].erase(solution[i].begin() + random_change);
      solution.push_back(aux);
      aux.clear();

    }
  }
  /*solution_value = 0;
  for(int i = 0; i< solution.size(); i++){
    if(solution[i].size() == 0){
      continue;
    }
    solution_value += dis_depot[solution[i][0]];
    if(solution[i].size() > 0){
      solution_value += dis_depot[solution[i][solution[i].size()-1]];
    }
    if(solution[i].size() != 1){
      for(int j = 0; j< solution[i].size()-1; j++){
        solution_value += dis_matrix[solution[i][j]][solution[i][j+1]];
      }
    }
  }*/

  
  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
  ((emili::vrp::VrpSolution*) base )->removeEmptyVehicles();
  if (empty_vehicle) {
    //std::cout << "fallo 5" << std::endl;
    ((emili::vrp::VrpSolution*) base )->removeEmptyVehicles();
    //std::cout << "fallo 5" << std::endl;
  } 
}

double emili::vrp::Shift_1_0_Neighborhood_With_Reparation::calculateDeltaSolutionValue(emili::Solution* base)
{
  
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  int current_customer = solution[next_move.first.first][next_move.first.second];
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  int orig_vehicle = next_move.first.first;
  int dest_vehicle = next_move.second.first;
  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  
  if (orig_vehicle == dest_vehicle) return(solution_value);
  
  // remove origin distances
  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[current_customer];
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][current_customer];
  }
  if (orig_post_customer == -1) {
    delta = delta - dis_depot[current_customer];
  } else {
    delta = delta - dis_matrix[current_customer][orig_post_customer];
  }
  //std::cout << "delta " << delta << std::endl;
  
  // add origin distances
  if (orig_prev_customer == -1) {
    //if both are -1 there is nothing to add
    if (orig_post_customer != -1) {
      delta = delta + dis_depot[orig_post_customer];
    } 
  } else {
    if (orig_post_customer == -1) {
      delta = delta + dis_depot[orig_prev_customer];
    } else {
      delta = delta + dis_matrix[orig_prev_customer][orig_post_customer];
    }
  }
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances
  if (dest_prev_customer == -1) {
    //If both are -1 there nothing to remove
    if (dest_post_customer != -1) {
      delta = delta - dis_depot[dest_post_customer];
    }
  } else {
    if (dest_post_customer == -1) {
      delta = delta - dis_depot[dest_prev_customer];
    } else {
      delta = delta - dis_matrix[dest_prev_customer][dest_post_customer];
    }
  }
  //std::cout << "delta " << delta << std::endl;
  
  // add destination distances
  if (dest_prev_customer == -1) {
    if (dest_post_customer == -1) {
      delta = delta + 2*dis_depot[current_customer];
    } else {
      delta = delta + dis_depot[current_customer] + dis_matrix[current_customer][dest_post_customer];
    }
  } else {
    if (dest_post_customer == -1) {
      delta = delta + dis_matrix[dest_prev_customer][current_customer] + dis_depot[current_customer];
    } else {
      delta = delta + dis_matrix[dest_prev_customer][current_customer] + dis_matrix[current_customer][dest_post_customer];
    }
  }

  new_value = solution_value + delta;
  //std::cout << "delta " << delta << " current " << solution_value << " new " << new_value << std::endl;
  
  return new_value;
}

void emili::vrp::Shift_1_0_Neighborhood_With_Reparation::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  //Set previous and posterior costuments
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[orig_vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[orig_vehicle].size()-1))
    orig_post_customer = solution[orig_vehicle][orig_index + 1];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[dest_vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_vehicle!=solution.size() && dest_index < solution[dest_vehicle].size())
    dest_post_customer = solution[dest_vehicle][dest_index];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}
void emili::vrp::Shift_1_0_Neighborhood_With_Reparation::repair(emili::Solution *base)
{
// esto va despues de make_move
  /*std::vector< std::vector<int> > solution_aux = *(solution);
  std::vector<int> used_capacity_aux =  *(used_capacity);
  std::vector<int> demands = pis.getDemands();
  std::vector <int> aux;
*/
  int vehicle_capacity = pis.getVehicleCapacity();
  int random_change;


  /*for(int i = 0; i < solution_aux.size(); i++){
    while(used_capacity_aux[i] >vehicle_capacity){
      random_change = emili::generateRandomNumber() % solution_aux[i].size();
      aux.push_back(solution_aux[i][random_change]);
      solution_aux[i].erase(solution_aux[i].begin() + random_change);
      used_capacity_aux[i] -= demands[random_change];
      used_capacity_aux.push_back(demands[random_change]);
      solution_aux.push_back(aux);
      aux.clear();
    }
  }*/



  //( (emili::vrp::VrpSolution*) base )->setSolutionValue(pis.computeObjectiveFunction(solution));
  //( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  //( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
  //((emili::vrp::VrpSolution*) base )->removeEmptyVehicles();
  //( (emili::vrp::VrpSolution*) base )->printSolutionRepresentation();
}

/*****************************************Shift_2_0_Neighborhood******************************************/

void emili::vrp::Shift_2_0_Neighborhood::calculateNeighborhoodSize(emili::Solution* base) 
{ 
  //Shift neighborhood considers all possible positions to move
  //moves must filtered based on the move
  int n_vehicles = ((emili::vrp::VrpSolution*) base)->getNVehicles();
  n_size = this->n_customers*(this->n_customers + n_vehicles + 1); 
};

bool emili::vrp::Shift_2_0_Neighborhood::isNextFeasible(emili::Solution* base)
{
  std::vector<int> used_capacities = ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int new_used_capacity = 0;
  std::vector<int> demands = pis.getDemands();

  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;

  //end of the vehicle (there no two customers)
  if (orig_index >= (solution[orig_vehicle].size()-1)){
	return false;
  }

  int current_customer1 = solution[orig_vehicle][orig_index];
  int current_customer2 = solution[orig_vehicle][orig_index+1];

  int dest_vehicle = next_move.second.first;
  if (dest_vehicle == solution.size()) {
    new_used_capacity = demands[current_customer1] + demands[current_customer2];
  } else {
    new_used_capacity = used_capacities[dest_vehicle] + demands[current_customer1] + demands[current_customer2];
  }

  if(new_used_capacity > pis.getVehicleCapacity()) {
    return false;
  }

  //Just changing of location the same route
  if (solution[orig_vehicle].size() == 2 && dest_vehicle == solution.size()){ 
    return false;
  }
  return true;
}

emili::Solution* emili::vrp::Shift_2_0_Neighborhood::random(Solution* currentSolution) 
{
  int orig_vehicle;
  int orig_index;
  int dest_vehicle;
  int dest_index;
  
  //std::cout << "random interoute" << std::endl;
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();
  int contador = 0;
  int flag = 0;
  do {
    //Get random customer indexes
    orig_vehicle = emili::generateRandomNumber() % solution.size();
    orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
    do {
      dest_vehicle = emili::generateRandomNumber() %  solution.size();
    } while (dest_vehicle==orig_vehicle);
    dest_index = emili::generateRandomNumber() % (solution[dest_vehicle].size()+1);
    next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    //std::cout << "random shift20 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << std::endl; 
    contador++;
    //std::cout<< "counter es " <<contador<< std::endl;
  } while ( !isNextFeasible(new_solution) && (contador <= 100) );
  if(contador < 100){
  	setAuxiliaryVariables(solution);
  	makeMove(new_solution);
  }
  return new_solution;
}

void emili::vrp::Shift_2_0_Neighborhood::setNextNeighborPos(emili::Solution* base)
{
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
  
  do {
    if (!isFirstTime) {
      if (c_iterations > (n_customers + solution.size()+1)) {
        orig_index++;
        if (orig_index >= solution[orig_vehicle].size()){
          orig_index = 0;
          orig_vehicle++;
          if (orig_vehicle == solution.size())
            orig_vehicle = 0;
        }
        c_iterations = 1;
      } else {
        dest_index++;
        if (dest_vehicle >= solution.size()) {
          //we assume one more vehicle here we check if
          //we are in this virtial vehicle
          dest_vehicle = 0;
          dest_index = 0;
        } else if (dest_index > solution[dest_vehicle].size()) {
          //rolling of the neighbors
          dest_vehicle++;
          dest_index = 0;
        }
      }
    } else {
      isFirstTime = false;
      if (orig_index >= solution[orig_vehicle].size()){
        orig_index = 0;
        orig_vehicle++;
      } 
      if (orig_vehicle >= solution.size()){
        orig_vehicle = 0;
        orig_index = 0;
      }
      if (dest_vehicle >= solution.size()) {
        dest_vehicle = 0;
        dest_index = 0;
      }
      if (dest_index > solution[dest_vehicle].size()){
        dest_index =0;
        dest_vehicle++;
      }
    }
    
    //std::cout << "shift20 c:" << current_customer << " o:" << orig_vehicle << "," << orig_index << " d:" << 
      //dest_vehicle << "," << dest_index << " step: " << c_iterations << "/" << n_customers + solution.size()+1 << "  " << n_iterations << "/" << n_size << std::endl;
    
    next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    setAuxiliaryVariables(solution);
    n_iterations++;
    c_iterations++;
    
  } while (dest_vehicle==orig_vehicle && n_iterations <= n_size);
}

void emili::vrp::Shift_2_0_Neighborhood::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();
  bool empty_vehicle = false;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int current_customer1 = solution[orig_vehicle][orig_index];
  int current_customer2 = solution[orig_vehicle][orig_index+1];
  //std::cout << "holi"<< std::endl;
  // remove target customers
  solution[orig_vehicle].erase(solution[orig_vehicle].begin() + orig_index);
  solution[orig_vehicle].erase(solution[orig_vehicle].begin() + orig_index);
  used_capacity[orig_vehicle] = used_capacity[orig_vehicle] - demands[current_customer1] - demands[current_customer2];

  if (solution[orig_vehicle].size() < 1){ 
	empty_vehicle = true;
  }
  // insert target customer
  if (dest_vehicle == solution.size()) {
    //new vehicle
    std::vector <int> n;
    n.push_back(current_customer1);
    n.push_back(current_customer2); 
    solution.push_back(n);
    used_capacity.push_back(demands[current_customer1] + demands[current_customer2]);

  } else {

    //solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index ), current_customer2);
    //solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index ), current_customer1);
    if(solution[dest_vehicle].size() < dest_index){
    	solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + solution[dest_vehicle].size()), current_customer2);
    	solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + solution[dest_vehicle].size()), current_customer1);
    }
    else{
    	solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index), current_customer2);
    	solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index), current_customer1);
    }
    used_capacity[dest_vehicle] = used_capacity[dest_vehicle] + demands[current_customer1] + demands[current_customer2];
  }
  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
  if (empty_vehicle) {
    ((emili::vrp::VrpSolution*) base )->removeEmptyVehicles();
  } 
}

double emili::vrp::Shift_2_0_Neighborhood::calculateDeltaSolutionValue(emili::Solution* base)
{
  
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  int current_customer1 = solution[next_move.first.first][next_move.first.second];
  int current_customer2 = solution[next_move.first.first][next_move.first.second+1];
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  /*std::cout << "posiciones" << std::endl;
  std::cout << next_move.first.first << std::endl;
  std::cout << next_move.first.second << std::endl;
  std::cout << next_move.second.first << std::endl;
  std::cout << next_move.second.second << std::endl;

  std::cout << "valores seteados" << std::endl;
  std::cout << pre_post_customers.first.first << std::endl;
  std::cout << pre_post_customers.first.second << std::endl;
  std::cout << pre_post_customers.second.first << std::endl;
  std::cout << pre_post_customers.second.second << std::endl;
  std::cout << std::endl;
  ((emili::vrp::VrpSolution*) base) ->printSolutionRepresentation();*/
  int orig_vehicle = next_move.first.first;
  int dest_vehicle = next_move.second.first;
  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  
  if (orig_vehicle == dest_vehicle) return(solution_value);
  
  // remove origin distances
  delta = delta - dis_matrix[current_customer1][current_customer2];
  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[current_customer1];
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][current_customer1];
  }
  if (orig_post_customer == -1) {
    delta = delta - dis_depot[current_customer2];
  } else {
    delta = delta - dis_matrix[current_customer2][orig_post_customer];
  }
  //std::cout << "delta " << delta << std::endl;
  
  // add origin distances
  if (orig_prev_customer == -1) {
    //if both are -1 there is nothing to add
    if (orig_post_customer != -1) {
      delta = delta + dis_depot[orig_post_customer];
    } 
  } else {
    if (orig_post_customer == -1) {
      delta = delta + dis_depot[orig_prev_customer];
    } else {
      delta = delta + dis_matrix[orig_prev_customer][orig_post_customer];
    }
  }
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances
  if (dest_prev_customer == -1) {
    //If both are -1 there nothing to remove
    if (dest_post_customer != -1) {
      delta = delta - dis_depot[dest_post_customer];
    }
  } else {
    if (dest_post_customer == -1) {
      delta = delta - dis_depot[dest_prev_customer];
    } else {
      delta = delta - dis_matrix[dest_prev_customer][dest_post_customer];
    }
  }
  //std::cout << "delta " << delta << std::endl;
  
  // add destination distances
  delta = delta + dis_matrix[current_customer1][current_customer2];
  if (dest_prev_customer == -1) {
    if (dest_post_customer == -1) {
      delta = delta + dis_depot[current_customer1] +  dis_depot[current_customer2];
    } else {
      delta = delta + dis_depot[current_customer1] + dis_matrix[current_customer2][dest_post_customer];
    }
  } else {
    if (dest_post_customer == -1) {
      delta = delta + dis_matrix[dest_prev_customer][current_customer1] + dis_depot[current_customer2];
    } else {
      delta = delta + dis_matrix[dest_prev_customer][current_customer1] + dis_matrix[current_customer2][dest_post_customer];
    }
  }
  
  //std::cout << "fdelta " << solution_value << "->"<< delta << std::endl;
  
  new_value = solution_value + delta;
  return new_value;
}

void emili::vrp::Shift_2_0_Neighborhood::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  //Set previous and posterior costuments
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[orig_vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[orig_vehicle].size()-2))
    orig_post_customer = solution[orig_vehicle][orig_index + 2];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[dest_vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_vehicle!=solution.size() && dest_index < solution[dest_vehicle].size())
    dest_post_customer = solution[dest_vehicle][dest_index];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}

/*****************************************Shift_2_0_Neighborhood_With_Reparation******************************************/

void emili::vrp::Shift_2_0_Neighborhood_With_Reparation::calculateNeighborhoodSize(emili::Solution* base) 
{ 
  //Shift neighborhood considers all possible positions to move
  //moves must filtered based on the move
  int n_vehicles = ((emili::vrp::VrpSolution*) base)->getNVehicles();
  n_size = this->n_customers*(this->n_customers + n_vehicles + 1); 
};

bool emili::vrp::Shift_2_0_Neighborhood_With_Reparation::isNextFeasible(emili::Solution* base)
{
  std::vector<int> used_capacities = ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int new_used_capacity = 0;
  std::vector<int> demands = pis.getDemands();

  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;

  //end of the vehicle (there no two customers)
  if (orig_index >= (solution[orig_vehicle].size()-1)){
	return false;
  }
  int current_customer1 = solution[orig_vehicle][orig_index];
  int current_customer2 = solution[orig_vehicle][orig_index+1];

  int dest_vehicle = next_move.second.first;
  if (dest_vehicle == solution.size()) {
    new_used_capacity = demands[current_customer1] + demands[current_customer2];
  } else {
    new_used_capacity = used_capacities[dest_vehicle] + demands[current_customer1] + demands[current_customer2];
  }

  if(new_used_capacity > pis.getVehicleCapacity()) {
    return false;
  }

  //Just changing of location the same route
  if (solution[orig_vehicle].size() == 2 && dest_vehicle == solution.size()){ 
    return false;
  }
  return true;
}

emili::Solution* emili::vrp::Shift_2_0_Neighborhood_With_Reparation::random(Solution* currentSolution) 
{
  int orig_vehicle;
  int orig_index;
  int dest_vehicle;
  int dest_index;
  
  //std::cout << "random interoute" << std::endl;
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();
  int contador = 0;
  int flag = 0;
  do {
    //Get random customer indexes
    orig_vehicle = emili::generateRandomNumber() % solution.size();
    orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
    do {
      dest_vehicle = emili::generateRandomNumber() %  solution.size();
      dest_index = emili::generateRandomNumber() % (solution[dest_vehicle].size()+1);
    } while (dest_vehicle==orig_vehicle );
    next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    contador++;
  } while ( (solution[orig_vehicle].size() - 1) - orig_index <= 2  );
  std::cout << "random shift20 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << std::endl;
  setAuxiliaryVariables(solution);
  makeMove(new_solution);
  repair(new_solution);

  return new_solution;
}

void emili::vrp::Shift_2_0_Neighborhood_With_Reparation::setNextNeighborPos(emili::Solution* base)
{
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
  
  do {
    if (!isFirstTime) {
      if (c_iterations > (n_customers + solution.size()+1)) {
        orig_index++;
        if (orig_index >= solution[orig_vehicle].size()){
          orig_index = 0;
          orig_vehicle++;
          if (orig_vehicle == solution.size())
            orig_vehicle = 0;
        }
        c_iterations = 1;
      } else {
        dest_index++;
        if (dest_vehicle >= solution.size()) {
          //we assume one more vehicle here we check if
          //we are in this virtial vehicle
          dest_vehicle = 0;
          dest_index = 0;
        } else if (dest_index > solution[dest_vehicle].size()) {
          //rolling of the neighbors
          dest_vehicle++;
          dest_index = 0;
        }
      }
    } else {
      isFirstTime = false;
      if (orig_index >= solution[orig_vehicle].size()){
        orig_index = 0;
        orig_vehicle++;
      } 
      if (orig_vehicle >= solution.size()){
        orig_vehicle = 0;
        orig_index = 0;
      }
      if (dest_vehicle >= solution.size()) {
        dest_vehicle = 0;
        dest_index = 0;
      }
      if (dest_index > solution[dest_vehicle].size()){
        dest_index =0;
        dest_vehicle++;
      }
    }
    
    //std::cout << "shift20 c:" << current_customer << " o:" << orig_vehicle << "," << orig_index << " d:" << 
      //dest_vehicle << "," << dest_index << " step: " << c_iterations << "/" << n_customers + solution.size()+1 << "  " << n_iterations << "/" << n_size << std::endl;
    
    next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    setAuxiliaryVariables(solution);
    n_iterations++;
    c_iterations++;
    
  } while (dest_vehicle==orig_vehicle && n_iterations <= n_size);
}

void emili::vrp::Shift_2_0_Neighborhood_With_Reparation::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();

  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();
  bool empty_vehicle = false;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int current_customer1 = solution[orig_vehicle][orig_index];
  int current_customer2 = solution[orig_vehicle][orig_index+1];

  // remove target customers
  solution[orig_vehicle].erase(solution[orig_vehicle].begin() + orig_index);
  solution[orig_vehicle].erase(solution[orig_vehicle].begin() + orig_index);
  used_capacity[orig_vehicle] = used_capacity[orig_vehicle] - demands[current_customer1] - demands[current_customer2];
  
  if (solution[orig_vehicle].size() < 1){ 
	empty_vehicle = true;
  }
  // insert target customer
  if (dest_vehicle == solution.size()) {
    //new vehicle
    std::vector <int> n;
    n.push_back(current_customer1);
    n.push_back(current_customer2); 
    solution.push_back(n);
    used_capacity.push_back(demands[current_customer1] + demands[current_customer2]);

  } else {

    //solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index ), current_customer2);
    //solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index ), current_customer1);
    if(solution[dest_vehicle].size() < dest_index){
    	solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + solution[dest_vehicle].size()), current_customer2);
    	solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + solution[dest_vehicle].size()), current_customer1);
    }
    else{
    	solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index), current_customer2);
    	solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index), current_customer1);
    }
    used_capacity[dest_vehicle] = used_capacity[dest_vehicle] + demands[current_customer1] + demands[current_customer2];
  }
  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
  if (empty_vehicle) {
    ((emili::vrp::VrpSolution*) base )->removeEmptyVehicles();
  } 
}

double emili::vrp::Shift_2_0_Neighborhood_With_Reparation::calculateDeltaSolutionValue(emili::Solution* base)
{
  
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  int current_customer1 = solution[next_move.first.first][next_move.first.second];
  int current_customer2 = solution[next_move.first.first][next_move.first.second+1];
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  int orig_vehicle = next_move.first.first;
  int dest_vehicle = next_move.second.first;
  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  
  if (orig_vehicle == dest_vehicle) return(solution_value);
  
  // remove origin distances

  delta = delta - dis_matrix[current_customer1][current_customer2];

  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[current_customer1];
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][current_customer1];
  }
  if (orig_post_customer == -1) {
    delta = delta - dis_depot[current_customer2];
  } else {
    delta = delta - dis_matrix[current_customer2][orig_post_customer];
  }
  //std::cout << "delta " << delta << std::endl;
  // add origin distances
  if (orig_prev_customer == -1) {
    //if both are -1 there is nothing to add
    if (orig_post_customer != -1) {
      delta = delta + dis_depot[orig_post_customer];
    } 
  } else {
    if (orig_post_customer == -1) {
      delta = delta + dis_depot[orig_prev_customer];
    } else {
      delta = delta + dis_matrix[orig_prev_customer][orig_post_customer];
    }
  }
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances

  if (dest_prev_customer == -1) {
    //If both are -1 there nothing to remove
    if (dest_post_customer != -1) {
      delta = delta - dis_depot[dest_post_customer];
    }
  } else {
    if (dest_post_customer == -1) {
      delta = delta - dis_depot[dest_prev_customer];
    } else {
      delta = delta - dis_matrix[dest_prev_customer][dest_post_customer];
    }
  }
  //std::cout << "delta " << delta << std::endl;
  
  // add destination distances

  delta = delta + dis_matrix[current_customer1][current_customer2];
  if (dest_prev_customer == -1) {
    if (dest_post_customer == -1) {
      delta = delta + dis_depot[current_customer1] +  dis_depot[current_customer2];
    } else {
      delta = delta + dis_depot[current_customer1] + dis_matrix[current_customer2][dest_post_customer];
    }
  } else {
    if (dest_post_customer == -1) {
      delta = delta + dis_matrix[dest_prev_customer][current_customer1] + dis_depot[current_customer2];
    } else {
      delta = delta + dis_matrix[dest_prev_customer][current_customer1] + dis_matrix[current_customer2][dest_post_customer];
    }
  }
  
  //std::cout << "fdelta " << solution_value << "->"<< delta << std::endl;
  
  new_value = solution_value + delta;
  return new_value;
}

void emili::vrp::Shift_2_0_Neighborhood_With_Reparation::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;

  //Set previous and posterior costuments
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[orig_vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[orig_vehicle].size()-2))
    orig_post_customer = solution[orig_vehicle][orig_index + 2];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[dest_vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_vehicle!=solution.size() && dest_index < solution[dest_vehicle].size())
    dest_post_customer = solution[dest_vehicle][dest_index];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}

void emili::vrp::Shift_2_0_Neighborhood_With_Reparation::repair(emili::Solution* base)
{
// esto va despues de make_move
  std::vector< std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector<int> demands = pis.getDemands();
  std::vector<std::vector<double>> DistanceMatrix = pis.getDistanceMatrix();
  std::vector<double> WarehousesDistance = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  int vehicle_capacity = pis.getVehicleCapacity();

  std::vector<int> temporal_solution;
  int temporal_customer = 0;
  double temporal_solution_value  = 0;
  double temporal_demand = 0;
  int empty;
  int random_change;
  for(int i = 0; i < solution.size(); i++){
    while(used_capacity[i] > vehicle_capacity){

      random_change = emili::generateRandomNumber() % solution[i].size();
      temporal_customer = solution[i][random_change];

      if(temporal_demand + demands[temporal_customer] > vehicle_capacity){
        solution.push_back(temporal_solution);
        used_capacity.push_back(temporal_demand);
        solution_value += temporal_solution_value;

        temporal_solution.clear();
        temporal_demand = 0;
        temporal_solution_value = 0;
      }
      /*if(random_change == 0){
        solution_value += - WarehousesDistance[temporal_customer] + WarehousesDistance[solution[i][1]];
        solution_value += - DistanceMatrix[solution[i][0]][solution[i][1]];
      }
      else if(random_change == solution[i].size() - 1){
        solution_value = solution_value - WarehousesDistance[temporal_customer];
        solution_value = solution_value + WarehousesDistance[solution[i][random_change -1]];
      }
      else{
        solution_value += - DistanceMatrix[solution[i][random_change] ][ solution[i][random_change - 1] ];
        solution_value += - DistanceMatrix[solution[i][random_change] ][ solution[i][random_change + 1] ];
        solution_value += - DistanceMatrix[solution[i][random_change - 1] ][ solution[i][random_change + 1] ];
      }*/
      used_capacity[i] = used_capacity[i] - demands[temporal_customer];
      solution[i].erase(solution[i].begin() + random_change);
      temporal_demand += demands[temporal_customer];
      temporal_solution.push_back(temporal_customer);

      /*if(temporal_solution.size() == 0){
        temporal_solution.push_back(temporal_customer);
        temporal_solution_value += 2*WarehousesDistance[temporal_customer];
      }
      else{
        temporal_solution.push_back(temporal_customer);
        temporal_solution_value = temporal_solution_value - WarehousesDistance[temporal_solution[temporal_solution.size() - 2]];
        temporal_solution_value = temporal_solution_value + WarehousesDistance[temporal_solution[temporal_solution.size() - 1]];
        temporal_solution_value = temporal_solution_value + DistanceMatrix[temporal_solution[temporal_solution.size() - 2]][temporal_solution[temporal_solution.size() - 1]];
      }*/

    }
  }

  solution.push_back(temporal_solution);
  used_capacity.push_back(temporal_demand);
  solution_value += temporal_solution_value;

  int i = 0;
  while(i < solution.size() ){
    
    
    if(solution[i].size() == 0){
      solution.erase(solution.begin() + i);
      used_capacity.erase(used_capacity.begin() + i);
      i = 0;
    }
    i++;
  }

  double fitness = 0;
  for(int i = 0; i < solution.size(); i++){
    for(int j = 0; j < solution[i].size();j++){
      if(j == 0 || j == solution.size() - 1){
        fitness += WarehousesDistance[solution[i][j]];
      }
      else{
        fitness += DistanceMatrix[j][j+1];
      }
    }
  }


  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(fitness);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
  //std::cout << "fitness " << solution_value << std::endl;


  //std::cout << "final" << std::endl;
}

/*************************************Swap_1_1_Neighborhood**************************************/

void emili::vrp::Swap_1_1_Neighborhood::calculateNeighborhoodSize(emili::Solution* base) 
{
  int customers = pis.getNcustomers();
  n_size = customers*customers;
}

emili::Solution* emili::vrp::Swap_1_1_Neighborhood::random(Solution* currentSolution) 
{
  int orig_vehicle;
  int orig_index;
  int dest_vehicle;
  int dest_index;
  int n_checked;
  int total_checked = 0;
  
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();
  
  calculateNeighborhoodSize(new_solution);

  //Get random customer indexes to start
  orig_vehicle = emili::generateRandomNumber() % solution.size();
  orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
  do {
    dest_vehicle = emili::generateRandomNumber() %  solution.size();
  } while (dest_vehicle==orig_vehicle);
  dest_index = emili::generateRandomNumber() % solution[dest_vehicle].size();
  next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
  
  //Cycle indexes
  n_checked = total_checked = 1;  
  while (!isNextFeasible(currentSolution)) {
    if (total_checked >= size()) 
      return(new_solution);
    if (n_checked == (n_customers-1)) {
      // move origin clients once check all destination
      orig_index++;
      if (orig_index == solution[orig_vehicle].size()) {
         orig_index = 0;
         orig_vehicle++;
         if (orig_vehicle == solution.size())
            orig_vehicle = 0;

      }
      n_checked = 0;
    } else {
      //move all destination clients first
      dest_index++;
      if (dest_index == solution[dest_vehicle].size()) {
         //rolling of the neighbors
         dest_vehicle++;
         dest_index = 0;
         if (dest_vehicle == solution.size())
            dest_vehicle=0;
      }
    }
    next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    n_checked++;
    total_checked++;
  }
  
  //std::cout << "random swap11 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << std::endl; 
  setAuxiliaryVariables(solution);
  makeMove(new_solution);
 
  return new_solution;
}

void emili::vrp::Swap_1_1_Neighborhood::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  //Set previous and posterior costuments
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[orig_vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[orig_vehicle].size()-1))
    orig_post_customer = solution[orig_vehicle][orig_index + 1];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[dest_vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_index < (solution[dest_vehicle].size()-1))
    dest_post_customer = solution[dest_vehicle][dest_index+1];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}

void emili::vrp::Swap_1_1_Neighborhood::setNextNeighborPos(emili::Solution* base)
{
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
  
  do {
    if (!isFirstTime) {
      if (c_iterations > n_customers) {
        //finished moves of a costumer
        orig_index++;
        if (orig_index == solution[orig_vehicle].size()){
          orig_index = 0;
          orig_vehicle++;
          if (orig_vehicle == solution.size())
            orig_vehicle = 0;
        }
        c_iterations = 1;
      } else {
        dest_index++;
        if (dest_index == solution[dest_vehicle].size()) {
          //rolling of the neighbors
          dest_vehicle++;
          dest_index = 0;
          if (dest_vehicle == solution.size())
            dest_vehicle = 0;
        }
      }
    } else {
      isFirstTime = false;
    }
    
   // std::cout << "swap11 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << " step: " 
              //<< c_iterations << "/" << n_customers << "  " << n_iterations << "/" << n_size << std::endl;
    
    next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    setAuxiliaryVariables(solution);
    n_iterations++;
    c_iterations++;
    
  } while (dest_vehicle==orig_vehicle);
  //std::cout<< "paso" <<std::endl;
}

bool emili::vrp::Swap_1_1_Neighborhood::isNextFeasible(emili::Solution* base)
{
  std::vector<int> used_capacities = ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int orig_used_capacity = 0;
  int dest_used_capacity = 0;
  std::vector<int> demands = pis.getDemands();
  
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int orig_customer = solution[orig_vehicle][orig_index];
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int dest_customer = solution[dest_vehicle][dest_index];

  if (orig_vehicle == dest_vehicle) return(false);

  orig_used_capacity = used_capacities[orig_vehicle] + demands[dest_customer] - demands[orig_customer];
  dest_used_capacity = used_capacities[dest_vehicle] + demands[orig_customer] - demands[dest_customer];

  if(orig_used_capacity > pis.getVehicleCapacity() || dest_used_capacity > pis.getVehicleCapacity()) {
    return false;
  }
  
  return true;
}

void emili::vrp::Swap_1_1_Neighborhood::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int orig_customer = solution[orig_vehicle][orig_index];
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int dest_customer = solution[dest_vehicle][dest_index];
  
  // change customer in origin
  solution[orig_vehicle][orig_index] = dest_customer;
  used_capacity[orig_vehicle] = used_capacity[orig_vehicle] - demands[orig_customer] + demands[dest_customer];
  
  // change customer in destination
  solution[dest_vehicle][dest_index] = orig_customer;
  used_capacity[dest_vehicle] = used_capacity[dest_vehicle] - demands[dest_customer] + demands[orig_customer];
  
  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);

  

}

double emili::vrp::Swap_1_1_Neighborhood::calculateDeltaSolutionValue(emili::Solution* base)
{
  
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  
  int orig_customer = solution[next_move.first.first][next_move.first.second];
  int dest_customer = solution[next_move.second.first][next_move.second.second];
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  int orig_vehicle = next_move.first.first;
  int dest_vehicle = next_move.second.first;
  
  //inter-route only
  if (orig_vehicle == dest_vehicle) 
    return(solution_value);
  

  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  // origin distances
  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[orig_customer];
    delta = delta +  dis_depot[dest_customer];
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][orig_customer];
    delta = delta + dis_matrix[orig_prev_customer][dest_customer];
  }
  if (orig_post_customer == -1) {
    delta = delta - dis_depot[orig_customer];
    delta = delta + dis_depot[dest_customer];
  } else {
    delta = delta - dis_matrix[orig_customer][orig_post_customer];
    delta = delta + dis_matrix[dest_customer][orig_post_customer];
  }
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances
  if (dest_prev_customer == -1) {
    delta = delta - dis_depot[dest_customer];
    delta = delta + dis_depot[orig_customer];
  } else {
    delta = delta - dis_matrix[dest_prev_customer][dest_customer];
    delta = delta + dis_matrix[dest_prev_customer][orig_customer];
  }
  
  if (dest_post_customer == -1) {
    delta = delta - dis_depot[dest_customer];
    delta = delta + dis_depot[orig_customer];
  } else {
    delta = delta - dis_matrix[dest_customer][dest_post_customer];
    delta = delta + dis_matrix[orig_customer][dest_post_customer];
  }
  
  //std::cout << "fdelta " << solution_value << "->"<< delta << std::endl;
  
  new_value = solution_value + delta;
  return new_value;
}

/*************************************Swap_1_1_Neighborhood_With_Reparation**************************************/

void emili::vrp::Swap_1_1_Neighborhood_With_Reparation::calculateNeighborhoodSize(emili::Solution* base) 
{
  int customers = pis.getNcustomers();
  n_size = customers*customers;
}

emili::Solution* emili::vrp::Swap_1_1_Neighborhood_With_Reparation::random(Solution* currentSolution) 
{
  int orig_vehicle;
  int orig_index;
  int dest_vehicle;
  int dest_index;
  int n_checked;
  int total_checked = 0;
  
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();
  
  calculateNeighborhoodSize(new_solution);

  //Get random customer indexes to start
  orig_vehicle = emili::generateRandomNumber() % solution.size();
  orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
  do {
    dest_vehicle = emili::generateRandomNumber() %  solution.size();
  } while (dest_vehicle==orig_vehicle);
  dest_index = emili::generateRandomNumber() % solution[dest_vehicle].size();
  next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
  
  //Cycle indexes
  n_checked = total_checked = 1;  
  //while (!isNextFeasible(currentSolution)) {
    if (total_checked >= size()) 
      return(new_solution);
    if (n_checked == (n_customers-1)) {
      // move origin clients once check all destination
      orig_index++;
      if (orig_index == solution[orig_vehicle].size()) {
         orig_index = 0;
         orig_vehicle++;
         if (orig_vehicle == solution.size())
            orig_vehicle = 0;

      }
      n_checked = 0;
    } else {
      //move all destination clients first
      dest_index++;
      if (dest_index == solution[dest_vehicle].size()) {
         //rolling of the neighbors
         dest_vehicle++;
         dest_index = 0;
         if (dest_vehicle == solution.size())
            dest_vehicle=0;
      }
    }
    next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    n_checked++;
    total_checked++;
  //}
  
  //std::cout << "random swap11 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << std::endl; 
  setAuxiliaryVariables(solution);
  makeMove(new_solution);
  repair(new_solution);
 
  return new_solution;
}

void emili::vrp::Swap_1_1_Neighborhood_With_Reparation::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  //Set previous and posterior costuments
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[orig_vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[orig_vehicle].size()-1))
    orig_post_customer = solution[orig_vehicle][orig_index + 1];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[dest_vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_index < (solution[dest_vehicle].size()-1))
    dest_post_customer = solution[dest_vehicle][dest_index+1];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}

void emili::vrp::Swap_1_1_Neighborhood_With_Reparation::setNextNeighborPos(emili::Solution* base)
{
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
  
  do {
    if (!isFirstTime) {
      if (c_iterations > n_customers) {
        //finished moves of a costumer
        orig_index++;
        if (orig_index == solution[orig_vehicle].size()){
          orig_index = 0;
          orig_vehicle++;
          if (orig_vehicle == solution.size())
            orig_vehicle = 0;
        }
        c_iterations = 1;
      } else {
        dest_index++;
        if (dest_index == solution[dest_vehicle].size()) {
          //rolling of the neighbors
          dest_vehicle++;
          dest_index = 0;
          if (dest_vehicle == solution.size())
            dest_vehicle = 0;
        }
      }
    } else {
      isFirstTime = false;
    }
    
   // std::cout << "swap11 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << " step: " 
              //<< c_iterations << "/" << n_customers << "  " << n_iterations << "/" << n_size << std::endl;
    
    next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    setAuxiliaryVariables(solution);
    n_iterations++;
    c_iterations++;
    
  } while (dest_vehicle==orig_vehicle);
  //std::cout<< "paso" <<std::endl;
}

bool emili::vrp::Swap_1_1_Neighborhood_With_Reparation::isNextFeasible(emili::Solution* base)
{
  std::vector<int> used_capacities = ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int orig_used_capacity = 0;
  int dest_used_capacity = 0;
  std::vector<int> demands = pis.getDemands();
  
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int orig_customer = solution[orig_vehicle][orig_index];
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int dest_customer = solution[dest_vehicle][dest_index];
  /*for(int i = 0; i < solution.size(); i++){
  	for(int j = 0; j < solution[i].size(); j++){
                std::cout<<solution[i][j]<<" ";
        }
        std::cout << std::endl;
  }*/
  if (orig_vehicle == dest_vehicle) return(false);

  orig_used_capacity = used_capacities[orig_vehicle] + demands[dest_customer] - demands[orig_customer];
  dest_used_capacity = used_capacities[dest_vehicle] + demands[orig_customer] - demands[dest_customer];

  if(orig_used_capacity > pis.getVehicleCapacity() || dest_used_capacity > pis.getVehicleCapacity()) {
    return false;
  }
  
  return true;
}

void emili::vrp::Swap_1_1_Neighborhood_With_Reparation::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int orig_customer = solution[orig_vehicle][orig_index];
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int dest_customer = solution[dest_vehicle][dest_index];
  
  // change customer in origin
  solution[orig_vehicle][orig_index] = dest_customer;
  used_capacity[orig_vehicle] = used_capacity[orig_vehicle] - demands[orig_customer] + demands[dest_customer];
  
  // change customer in destination
  solution[dest_vehicle][dest_index] = orig_customer;
  used_capacity[dest_vehicle] = used_capacity[dest_vehicle] - demands[dest_customer] + demands[orig_customer];
  
  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
}

double emili::vrp::Swap_1_1_Neighborhood_With_Reparation::calculateDeltaSolutionValue(emili::Solution* base)
{
  
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  
  int orig_customer = solution[next_move.first.first][next_move.first.second];
  int dest_customer = solution[next_move.second.first][next_move.second.second];
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  int orig_vehicle = next_move.first.first;
  int dest_vehicle = next_move.second.first;
  
  //inter-route only
  if (orig_vehicle == dest_vehicle) 
    return(solution_value);
  

  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  // origin distances
  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[orig_customer];
    delta = delta +  dis_depot[dest_customer];
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][orig_customer];
    delta = delta + dis_matrix[orig_prev_customer][dest_customer];
  }
  if (orig_post_customer == -1) {
    delta = delta - dis_depot[orig_customer];
    delta = delta + dis_depot[dest_customer];
  } else {
    delta = delta - dis_matrix[orig_customer][orig_post_customer];
    delta = delta + dis_matrix[dest_customer][orig_post_customer];
  }
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances
  if (dest_prev_customer == -1) {
    delta = delta - dis_depot[dest_customer];
    delta = delta + dis_depot[orig_customer];
  } else {
    delta = delta - dis_matrix[dest_prev_customer][dest_customer];
    delta = delta + dis_matrix[dest_prev_customer][orig_customer];
  }
  
  if (dest_post_customer == -1) {
    delta = delta - dis_depot[dest_customer];
    delta = delta + dis_depot[orig_customer];
  } else {
    delta = delta - dis_matrix[dest_customer][dest_post_customer];
    delta = delta + dis_matrix[orig_customer][dest_post_customer];
  }
  
  //std::cout << "fdelta " << solution_value << "->"<< delta << std::endl;
  
  new_value = solution_value + delta;
  return new_value;
}


void emili::vrp::Swap_1_1_Neighborhood_With_Reparation::repair(emili::Solution* base)
{
// esto va despues de make_move
  std::vector< std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector<int> demands = pis.getDemands();
  std::vector<std::vector<double>> DistanceMatrix = pis.getDistanceMatrix();
  std::vector<double> WarehousesDistance = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  int vehicle_capacity = pis.getVehicleCapacity();

  std::vector<int> temporal_solution;
  int temporal_customer = 0;
  double temporal_solution_value  = 0;
  double temporal_demand = 0;
  int empty;
  int random_change;
  for(int i = 0; i < solution.size(); i++){
    while(used_capacity[i] > vehicle_capacity){

      random_change = emili::generateRandomNumber() % solution[i].size();
      temporal_customer = solution[i][random_change];

      if(temporal_demand + demands[temporal_customer] > vehicle_capacity){
        solution.push_back(temporal_solution);
        used_capacity.push_back(temporal_demand);
        solution_value += temporal_solution_value;

        temporal_solution.clear();
        temporal_demand = 0;
        temporal_solution_value = 0;
      }
      /*if(random_change == 0){
        solution_value += - WarehousesDistance[temporal_customer] + WarehousesDistance[solution[i][1]];
        solution_value += - DistanceMatrix[solution[i][0]][solution[i][1]];
      }
      else if(random_change == solution[i].size() - 1){
        solution_value = solution_value - WarehousesDistance[temporal_customer];
        solution_value = solution_value + WarehousesDistance[solution[i][random_change -1]];
      }
      else{
        solution_value += - DistanceMatrix[solution[i][random_change] ][ solution[i][random_change - 1] ];
        solution_value += - DistanceMatrix[solution[i][random_change] ][ solution[i][random_change + 1] ];
        solution_value += - DistanceMatrix[solution[i][random_change - 1] ][ solution[i][random_change + 1] ];
      }*/
      used_capacity[i] = used_capacity[i] - demands[temporal_customer];
      solution[i].erase(solution[i].begin() + random_change);
      temporal_demand += demands[temporal_customer];
      temporal_solution.push_back(temporal_customer);

      /*if(temporal_solution.size() == 0){
        temporal_solution.push_back(temporal_customer);
        temporal_solution_value += 2*WarehousesDistance[temporal_customer];
      }
      else{
        temporal_solution.push_back(temporal_customer);
        temporal_solution_value = temporal_solution_value - WarehousesDistance[temporal_solution[temporal_solution.size() - 2]];
        temporal_solution_value = temporal_solution_value + WarehousesDistance[temporal_solution[temporal_solution.size() - 1]];
        temporal_solution_value = temporal_solution_value + DistanceMatrix[temporal_solution[temporal_solution.size() - 2]][temporal_solution[temporal_solution.size() - 1]];
      }*/

    }
  }

  solution.push_back(temporal_solution);
  used_capacity.push_back(temporal_demand);
  solution_value += temporal_solution_value;

  int i = 0;
  while(i < solution.size() ){
    
    
    if(solution[i].size() == 0){
      solution.erase(solution.begin() + i);
      used_capacity.erase(used_capacity.begin() + i);
      i = 0;
    }
    i++;
  }

  double fitness = 0;
  for(int i = 0; i < solution.size(); i++){
    for(int j = 0; j < solution[i].size();j++){
      if(j == 0 || j == solution.size() - 1){
        fitness += WarehousesDistance[solution[i][j]];
      }
      else{
        fitness += DistanceMatrix[j][j+1];
      }
    }
  }


  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(fitness);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
  //std::cout << "fitness " << solution_value << std::endl;


  //std::cout << "final" << std::endl;
}



/*************************************Swap_2_1_Neighborhood*********************************/
 
void emili::vrp::Swap_2_1_Neighborhood::calculateNeighborhoodSize(emili::Solution* base) 
{
  int customers = pis.getNcustomers();
  n_size = customers * customers;
}

void emili::vrp::Swap_2_1_Neighborhood::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  /*std::cout  << std::endl;
  std::cout  << std::endl;
  std::cout << "vehiculo origen " << orig_vehicle << std::endl;
  std::cout << "indice origen " << orig_index -1<< std::endl;
  std::cout  << std::endl;
  std::cout << "vehiculo destino " << dest_vehicle << std::endl;
  std::cout << "indice destino " << dest_index -1<< std::endl;
  std::cout  << std::endl;
  std::cout << "tamano vehiculo origen " << solution[orig_vehicle].size()-2 << std::endl;
  std::cout << "tamano vehiculo destino " << solution[dest_vehicle].size()-1 << std::endl;
  std::cout << "lugar fallo? " << solution[orig_vehicle][orig_index - 1] << std::endl;
  std::cout << "lugar fallo2? " << solution[dest_vehicle][dest_index - 1] << std::endl;
  std::cout  << std::endl;
  std::cout  << std::endl;*/
  //Set previous and posterior customers
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[orig_vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[orig_vehicle].size()-2))
    orig_post_customer = solution[orig_vehicle][orig_index + 2];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[dest_vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_index < (solution[dest_vehicle].size()-1))
    dest_post_customer = solution[dest_vehicle][dest_index+1];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}

emili::Solution* emili::vrp::Swap_2_1_Neighborhood::random(Solution* currentSolution) 
{
  int orig_vehicle;
  int orig_index;
  int dest_vehicle;
  int dest_index;
  int n_checked;
  int total_checked = 0;
  
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();
  
  calculateNeighborhoodSize(new_solution);

  //Get random customer indexes to start
  orig_vehicle = emili::generateRandomNumber() % solution.size();
  orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
  do {
    dest_vehicle = emili::generateRandomNumber() %  solution.size();
  } while (dest_vehicle==orig_vehicle);
  dest_index = emili::generateRandomNumber() % solution[dest_vehicle].size();
  next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
  
  //Cycle indexes
  n_checked = total_checked = 1;  
  while (!isNextFeasible(currentSolution)) {
    if (total_checked >= size()) 
      return(new_solution);
    if (n_checked == (n_customers-1)) {
      // move origin clients once check all destination
      orig_index++;
      if (orig_index == solution[orig_vehicle].size()) {
         orig_index = 0;
         orig_vehicle++;
         if (orig_vehicle == solution.size())
            orig_vehicle = 0;

      }
      n_checked = 0;
    } else {
      //move all destination clients first
      dest_index++;
      if (dest_index == solution[dest_vehicle].size()) {
         //rolling of the neighbors
         dest_vehicle++;
         dest_index = 0;
         if (dest_vehicle == solution.size())
            dest_vehicle=0;
      }
    }
    next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    n_checked++;
    total_checked++;
  }
  
  //std::cout << "random swap21 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << std::endl; 
  //setAuxiliaryVariables(new_solution);
  setAuxiliaryVariables(solution);
  makeMove(new_solution);
 
  return new_solution;
}

void emili::vrp::Swap_2_1_Neighborhood::setNextNeighborPos(emili::Solution* base)
{
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
 
  do {
    if (!isFirstTime) {
      if (c_iterations > n_customers) {
        orig_index++;
        if (orig_index >= solution[orig_vehicle].size() ){
          orig_index = 0;
          orig_vehicle++;
          if (orig_vehicle == solution.size())
            orig_vehicle = 0;
        }
	//orig_index++;
        c_iterations = 1;
      }
      else {
        dest_index++;
        if (dest_index == solution[dest_vehicle].size()) {
          //rolling of the neighbors
          dest_vehicle++;
          dest_index = 0;
          if (dest_vehicle == solution.size())
            dest_vehicle=0;
        }
      }
    } else {
      next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
      setAuxiliaryVariables(solution);
      isFirstTime = false;
    }
    //std::cout << "swap21 o:" << orig_vehicle << "," << orig_index << "  d:" << dest_vehicle << "," << dest_index << 
                //" step: " << c_iterations << "/" << n_customers << "  " << n_iterations << "/" << n_size << std::endl;
    
    //next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    //setAuxiliaryVariables(solution);
    n_iterations++;
    c_iterations++;
    
  } while (dest_vehicle==orig_vehicle);
  next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
  setAuxiliaryVariables(solution);
}

bool emili::vrp::Swap_2_1_Neighborhood::isNextFeasible(emili::Solution* base)
{
  std::vector<int> used_capacities = ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int orig_used_capacity = 0;
  int dest_used_capacity = 0;
  std::vector<int> demands = pis.getDemands();
  
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  if (orig_vehicle == dest_vehicle) return(false);

  if (orig_index >= (solution[orig_vehicle].size()-1)) return(false);
  
  int orig_customer1 = solution[orig_vehicle][orig_index];
  int orig_customer2 = solution[orig_vehicle][orig_index+1];
  int dest_customer = solution[dest_vehicle][dest_index];
  
  
  orig_used_capacity = used_capacities[orig_vehicle] + demands[dest_customer] - demands[orig_customer1] - demands[orig_customer2];
  dest_used_capacity = used_capacities[dest_vehicle] + demands[orig_customer1] + demands[orig_customer2] - demands[dest_customer];

  if(orig_used_capacity > pis.getVehicleCapacity() || dest_used_capacity > pis.getVehicleCapacity()) {
    return(false);
  }
  
  return true;
}

void emili::vrp::Swap_2_1_Neighborhood::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();

  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int orig_customer1 = solution[orig_vehicle][orig_index];
  int orig_customer2 = solution[orig_vehicle][orig_index+1];
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int dest_customer = solution[dest_vehicle][dest_index];
  // change customers in origin
  solution[orig_vehicle][orig_index] = dest_customer;
  solution[orig_vehicle].erase(solution[orig_vehicle].begin() + orig_index+1);
  used_capacity[orig_vehicle] = used_capacity[orig_vehicle] - demands[orig_customer1] - demands[orig_customer2] + demands[dest_customer];

  // change customer in destination
  solution[dest_vehicle][dest_index] = orig_customer1;
  solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index+1), orig_customer2);
  used_capacity[dest_vehicle] = used_capacity[dest_vehicle] + demands[orig_customer1] + demands[orig_customer2] - demands[dest_customer];

  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);


}

double emili::vrp::Swap_2_1_Neighborhood::calculateDeltaSolutionValue(emili::Solution* base)
{
  
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  
  int orig_customer1 = solution[next_move.first.first][next_move.first.second];
  int orig_customer2 = solution[next_move.first.first][next_move.first.second+1];
  int dest_customer = solution[next_move.second.first][next_move.second.second];
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  int orig_vehicle = next_move.first.first;
  int dest_vehicle = next_move.second.first;
  
  //inter-route only
  if (orig_vehicle == dest_vehicle) 
    return(solution_value);
  
  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  // origin distances
  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[orig_customer1];
    delta = delta +  dis_depot[dest_customer];
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][orig_customer1];
    delta = delta + dis_matrix[orig_prev_customer][dest_customer];
  }
  if (orig_post_customer == -1) {
    delta = delta - dis_depot[orig_customer2];
    delta = delta + dis_depot[dest_customer];
  } else {
    delta = delta - dis_matrix[orig_customer2][orig_post_customer];
    delta = delta + dis_matrix[dest_customer][orig_post_customer];
  }
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances
  if (dest_prev_customer == -1) {
    delta = delta - dis_depot[dest_customer];
    delta = delta + dis_depot[orig_customer1];
  } else {
    delta = delta - dis_matrix[dest_prev_customer][dest_customer];
    delta = delta + dis_matrix[dest_prev_customer][orig_customer1];
  }
  
  if (dest_post_customer == -1) {
    delta = delta - dis_depot[dest_customer];
    delta = delta + dis_depot[orig_customer2];
  } else {
    delta = delta - dis_matrix[dest_customer][dest_post_customer];
    delta = delta + dis_matrix[orig_customer2][dest_post_customer];
  }
  
  //std::cout << "fdelta " << solution_value << "->"<< delta << std::endl;
  
  new_value = solution_value + delta;
  return new_value;
}

/*************************************Swap_2_1_Neighborhood_With_Reparation*********************************/
 
void emili::vrp::Swap_2_1_Neighborhood_With_Reparation::calculateNeighborhoodSize(emili::Solution* base) 
{
  int customers = pis.getNcustomers();
  n_size = customers * customers;
}

void emili::vrp::Swap_2_1_Neighborhood_With_Reparation::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  /*std::cout  << std::endl;
  std::cout  << std::endl;
  std::cout << "vehiculo origen " << orig_vehicle << std::endl;
  std::cout << "indice origen " << orig_index -1<< std::endl;
  std::cout  << std::endl;
  std::cout << "vehiculo destino " << dest_vehicle << std::endl;
  std::cout << "indice destino " << dest_index -1<< std::endl;
  std::cout  << std::endl;
  std::cout << "tamano vehiculo origen " << solution[orig_vehicle].size()-2 << std::endl;
  std::cout << "tamano vehiculo destino " << solution[dest_vehicle].size()-1 << std::endl;
  std::cout << "lugar fallo? " << solution[orig_vehicle][orig_index - 1] << std::endl;
  std::cout << "lugar fallo2? " << solution[dest_vehicle][dest_index - 1] << std::endl;
  std::cout  << std::endl;
  std::cout  << std::endl;*/
  //Set previous and posterior customers
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[orig_vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[orig_vehicle].size()-2))
    orig_post_customer = solution[orig_vehicle][orig_index + 2];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[dest_vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_index < (solution[dest_vehicle].size()-1))
    dest_post_customer = solution[dest_vehicle][dest_index+1];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}

emili::Solution* emili::vrp::Swap_2_1_Neighborhood_With_Reparation::random(Solution* currentSolution) 
{
  int orig_vehicle;
  int orig_index;
  int dest_vehicle;
  int dest_index;
  int n_checked;
  int total_checked = 0;
  
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();
  
  calculateNeighborhoodSize(new_solution);

  //Get random customer indexes to start
  /*orig_vehicle = emili::generateRandomNumber() % solution.size();
  orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
  do {
    dest_vehicle = emili::generateRandomNumber() %  solution.size();
  } while (dest_vehicle==orig_vehicle);
  dest_index = emili::generateRandomNumber() % solution[dest_vehicle].size();
  next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));*/
  
  //Cycle indexes
  n_checked = total_checked = 1;  
  //while ( (solution[orig_vehicle].size() - 1 - orig_index < 2) && (solution[dest_vehicle].size() - 1 - dest_index < 1)  ) {
  do{
    /*if (total_checked >= size()) 
      return(new_solution);
    if (n_checked == (n_customers-1)) {
      // move origin clients once check all destination
      orig_index++;
      if (orig_index == solution[orig_vehicle].size()) {
         orig_index = 0;
         orig_vehicle++;
         if (orig_vehicle == solution.size())
            orig_vehicle = 0;

      }
      n_checked = 0;
    } else {
      //move all destination clients first
      dest_index++;
      if (dest_index == solution[dest_vehicle].size()) {
         //rolling of the neighbors
         dest_vehicle++;
         dest_index = 0;
         if (dest_vehicle == solution.size())
            dest_vehicle=0;
      }
    }
    next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    n_checked++;
    total_checked++;*/
    orig_vehicle = emili::generateRandomNumber() % solution.size();
    orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
    do {
      dest_vehicle = emili::generateRandomNumber() %  solution.size();
    } while (dest_vehicle==orig_vehicle);
    dest_index = emili::generateRandomNumber() % solution[dest_vehicle].size();
    next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
  } while( (solution[orig_vehicle].size() - 1 - orig_index < 2) || (solution[dest_vehicle].size() - 1 - dest_index < 1)  );
  //}
  
  std::cout << "random swap21 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << std::endl; 
  //((emili::vrp::VrpSolution*) currentSolution) -> printSolutionRepresentation();
  //setAuxiliaryVariables(new_solution);
  std::cout << "los datos son " << std::endl;
  std::cout << solution[orig_vehicle].size() << std::endl;
  std::cout << orig_index << std::endl;
  std::cout << "la resta es " << (solution[orig_vehicle].size() - 1) - orig_index << std::endl;
  std::cout << solution[dest_vehicle].size() << std::endl;
  std::cout << dest_index << std::endl;
  std::cout << "la resta es " << (solution[dest_vehicle].size() - 1) - dest_index << std::endl;
  std::cout << std::endl;
  setAuxiliaryVariables(solution);
  makeMove(new_solution);
  repair(new_solution);
  return new_solution;
}

void emili::vrp::Swap_2_1_Neighborhood_With_Reparation::setNextNeighborPos(emili::Solution* base)
{
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
 
  do {
    if (!isFirstTime) {
      if (c_iterations > n_customers) {
        orig_index++;
        if (orig_index >= solution[orig_vehicle].size() ){
          orig_index = 0;
          orig_vehicle++;
          if (orig_vehicle == solution.size())
            orig_vehicle = 0;
        }
	//orig_index++;
        c_iterations = 1;
      }
      else {
        dest_index++;
        if (dest_index == solution[dest_vehicle].size()) {
          //rolling of the neighbors
          dest_vehicle++;
          dest_index = 0;
          if (dest_vehicle == solution.size())
            dest_vehicle=0;
        }
      }
    } else {
      next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
      setAuxiliaryVariables(solution);
      isFirstTime = false;
    }
    //std::cout << "swap21 o:" << orig_vehicle << "," << orig_index << "  d:" << dest_vehicle << "," << dest_index << 
                //" step: " << c_iterations << "/" << n_customers << "  " << n_iterations << "/" << n_size << std::endl;
    
    //next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    //setAuxiliaryVariables(solution);
    n_iterations++;
    c_iterations++;
    
  } while (dest_vehicle==orig_vehicle);
  next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
  setAuxiliaryVariables(solution);
}

bool emili::vrp::Swap_2_1_Neighborhood_With_Reparation::isNextFeasible(emili::Solution* base)
{
  std::vector<int> used_capacities = ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int orig_used_capacity = 0;
  int dest_used_capacity = 0;
  std::vector<int> demands = pis.getDemands();
  
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  if (orig_vehicle == dest_vehicle) return(false);

  if (orig_index >= (solution[orig_vehicle].size()-1)) return(false);
  
  int orig_customer1 = solution[orig_vehicle][orig_index];
  int orig_customer2 = solution[orig_vehicle][orig_index+1];
  int dest_customer = solution[dest_vehicle][dest_index];
  
  
  orig_used_capacity = used_capacities[orig_vehicle] + demands[dest_customer] - demands[orig_customer1] - demands[orig_customer2];
  dest_used_capacity = used_capacities[dest_vehicle] + demands[orig_customer1] + demands[orig_customer2] - demands[dest_customer];

  if(orig_used_capacity > pis.getVehicleCapacity() || dest_used_capacity > pis.getVehicleCapacity()) {
    return(false);
  }
  
  return true;
}

void emili::vrp::Swap_2_1_Neighborhood_With_Reparation::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();

  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int orig_customer1 = solution[orig_vehicle][orig_index];
  int orig_customer2 = solution[orig_vehicle][orig_index+1];
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int dest_customer = solution[dest_vehicle][dest_index];
  // change customers in origin
  solution[orig_vehicle][orig_index] = dest_customer;
  solution[orig_vehicle].erase(solution[orig_vehicle].begin() + orig_index+1);
  used_capacity[orig_vehicle] = used_capacity[orig_vehicle] - demands[orig_customer1] - demands[orig_customer2] + demands[dest_customer];

  // change customer in destination
  solution[dest_vehicle][dest_index] = orig_customer1;
  solution[dest_vehicle].emplace((solution[dest_vehicle].begin() + dest_index+1), orig_customer2);
  used_capacity[dest_vehicle] = used_capacity[dest_vehicle] + demands[orig_customer1] + demands[orig_customer2] - demands[dest_customer];

  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);


}

double emili::vrp::Swap_2_1_Neighborhood_With_Reparation::calculateDeltaSolutionValue(emili::Solution* base)
{
  
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  
  int orig_customer1 = solution[next_move.first.first][next_move.first.second];
  int orig_customer2 = solution[next_move.first.first][next_move.first.second+1];
  int dest_customer = solution[next_move.second.first][next_move.second.second];
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  int orig_vehicle = next_move.first.first;
  int dest_vehicle = next_move.second.first;
  
  //inter-route only
  if (orig_vehicle == dest_vehicle) 
    return(solution_value);
  
  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  // origin distances
  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[orig_customer1];
    delta = delta +  dis_depot[dest_customer];
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][orig_customer1];
    delta = delta + dis_matrix[orig_prev_customer][dest_customer];
  }
  if (orig_post_customer == -1) {
    delta = delta - dis_depot[orig_customer2];
    delta = delta + dis_depot[dest_customer];
  } else {
    delta = delta - dis_matrix[orig_customer2][orig_post_customer];
    delta = delta + dis_matrix[dest_customer][orig_post_customer];
  }
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances
  if (dest_prev_customer == -1) {
    delta = delta - dis_depot[dest_customer];
    delta = delta + dis_depot[orig_customer1];
  } else {
    delta = delta - dis_matrix[dest_prev_customer][dest_customer];
    delta = delta + dis_matrix[dest_prev_customer][orig_customer1];
  }
  
  if (dest_post_customer == -1) {
    delta = delta - dis_depot[dest_customer];
    delta = delta + dis_depot[orig_customer2];
  } else {
    delta = delta - dis_matrix[dest_customer][dest_post_customer];
    delta = delta + dis_matrix[orig_customer2][dest_post_customer];
  }
  
  //std::cout << "fdelta " << solution_value << "->"<< delta << std::endl;
  
  new_value = solution_value + delta;
  return new_value;
}

void emili::vrp::Swap_2_1_Neighborhood_With_Reparation::repair(emili::Solution* base)
{
// esto va despues de make_move
  std::vector< std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector<int> demands = pis.getDemands();
  std::vector<std::vector<double>> DistanceMatrix = pis.getDistanceMatrix();
  std::vector<double> WarehousesDistance = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  int vehicle_capacity = pis.getVehicleCapacity();

  std::vector<int> temporal_solution;
  int temporal_customer = 0;
  double temporal_solution_value  = 0;
  double temporal_demand = 0;
  int empty;
  int random_change;
  for(int i = 0; i < solution.size(); i++){
    while(used_capacity[i] > vehicle_capacity){

      random_change = emili::generateRandomNumber() % solution[i].size();
      temporal_customer = solution[i][random_change];

      if(temporal_demand + demands[temporal_customer] > vehicle_capacity){
        solution.push_back(temporal_solution);
        used_capacity.push_back(temporal_demand);
        solution_value += temporal_solution_value;

        temporal_solution.clear();
        temporal_demand = 0;
        temporal_solution_value = 0;
      }
      /*if(random_change == 0){
        solution_value += - WarehousesDistance[temporal_customer] + WarehousesDistance[solution[i][1]];
        solution_value += - DistanceMatrix[solution[i][0]][solution[i][1]];
      }
      else if(random_change == solution[i].size() - 1){
        solution_value = solution_value - WarehousesDistance[temporal_customer];
        solution_value = solution_value + WarehousesDistance[solution[i][random_change -1]];
      }
      else{
        solution_value += - DistanceMatrix[solution[i][random_change] ][ solution[i][random_change - 1] ];
        solution_value += - DistanceMatrix[solution[i][random_change] ][ solution[i][random_change + 1] ];
        solution_value += - DistanceMatrix[solution[i][random_change - 1] ][ solution[i][random_change + 1] ];
      }*/
      used_capacity[i] = used_capacity[i] - demands[temporal_customer];
      solution[i].erase(solution[i].begin() + random_change);
      temporal_demand += demands[temporal_customer];
      temporal_solution.push_back(temporal_customer);

      /*if(temporal_solution.size() == 0){
        temporal_solution.push_back(temporal_customer);
        temporal_solution_value += 2*WarehousesDistance[temporal_customer];
      }
      else{
        temporal_solution.push_back(temporal_customer);
        temporal_solution_value = temporal_solution_value - WarehousesDistance[temporal_solution[temporal_solution.size() - 2]];
        temporal_solution_value = temporal_solution_value + WarehousesDistance[temporal_solution[temporal_solution.size() - 1]];
        temporal_solution_value = temporal_solution_value + DistanceMatrix[temporal_solution[temporal_solution.size() - 2]][temporal_solution[temporal_solution.size() - 1]];
      }*/

    }
  }

  solution.push_back(temporal_solution);
  used_capacity.push_back(temporal_demand);
  solution_value += temporal_solution_value;

  int i = 0;
  while(i < solution.size() ){
    
    
    if(solution[i].size() == 0){
      solution.erase(solution.begin() + i);
      used_capacity.erase(used_capacity.begin() + i);
      i = 0;
    }
    i++;
  }

  double fitness = 0;
  for(int i = 0; i < solution.size(); i++){
    for(int j = 0; j < solution[i].size();j++){
      if(j == 0 || j == solution.size() - 1){
        fitness += WarehousesDistance[solution[i][j]];
      }
      else{
        fitness += DistanceMatrix[j][j+1];
      }
    }
  }


  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(fitness);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
  //std::cout << "fitness " << solution_value << std::endl;


  //std::cout << "final" << std::endl;
}


/*****************************************Swap_2_2_Neighborhood**********************************/

void emili::vrp::Swap_2_2_Neighborhood::calculateNeighborhoodSize(emili::Solution* base) 
{
  int customers = pis.getNcustomers();
  n_size = customers*customers;
}

void emili::vrp::Swap_2_2_Neighborhood::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  //Set previous and posterior costuments
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[orig_vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[orig_vehicle].size()-2))
    orig_post_customer = solution[orig_vehicle][orig_index + 2];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[dest_vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_index < (solution[dest_vehicle].size()-2))
    dest_post_customer = solution[dest_vehicle][dest_index + 2];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}

emili::Solution* emili::vrp::Swap_2_2_Neighborhood::random(Solution* currentSolution) 
{
  int orig_vehicle;
  int orig_index;
  int dest_vehicle;
  int dest_index;
  int n_checked;
  int total_checked = 0;
  
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();
  
  calculateNeighborhoodSize(new_solution);

  //Get random customer indexes to start
  orig_vehicle = emili::generateRandomNumber() % solution.size();
  orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
  do {
    dest_vehicle = emili::generateRandomNumber() %  solution.size();
  } while (dest_vehicle==orig_vehicle);
  dest_index = emili::generateRandomNumber() % solution[dest_vehicle].size();
  next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
  
  //Cycle indexes
  n_checked = total_checked = 1;  
  while (!isNextFeasible(currentSolution)) {
    if (total_checked >= size()) 
      return(new_solution);
    if (n_checked == (n_customers-1)) {
      // move origin clients once check all destination
      orig_index++;
      if (orig_index == solution[orig_vehicle].size()) {
         orig_index = 0;
         orig_vehicle++;
         if (orig_vehicle == solution.size())
            orig_vehicle = 0;

      }
      n_checked = 0;
    } else {
      //move all destination clients first
      dest_index++;
      if (dest_index == solution[dest_vehicle].size()) {
         //rolling of the neighbors
         dest_vehicle++;
         dest_index = 0;
         if (dest_vehicle == solution.size())
            dest_vehicle=0;
      }
    }
    next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    n_checked++;
    total_checked++;
  }


 


  setAuxiliaryVariables(solution);
  makeMove(new_solution);
 
  return new_solution;
}

void emili::vrp::Swap_2_2_Neighborhood::setNextNeighborPos(emili::Solution* base)
{
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
  
  do {
    if (!isFirstTime) {
      if (c_iterations > n_customers) {
        //finished moves of a costumer
        orig_index++;
        if (orig_index == solution[orig_vehicle].size()){
          orig_index = 0;
          orig_vehicle++;
          if (orig_vehicle == solution.size())
            orig_vehicle = 0;
        }
	/*if(orig_vehicle == dest_vehicle && (dest_vehicle != solution.size())){
		dest_vehicle++;
		dest_index = 0;
	}*/
        c_iterations = 1;
      } else {
        dest_index++;
        if (dest_index == solution[dest_vehicle].size()) {
          //rolling of the neighbors
          dest_vehicle++;
          dest_index = 0;
          if (dest_vehicle == solution.size())
            dest_vehicle=0;
        }
      }
    } else {
      isFirstTime = false;
    }
    
    //std::cout << "swap22 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << 
                // " step: " << c_iterations << "/" << n_customers << "  " << n_iterations << "/" << n_size << std::endl;
    
    next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    setAuxiliaryVariables(solution);
    n_iterations++;
    c_iterations++;
    
  } while (dest_vehicle==orig_vehicle);
}

bool emili::vrp::Swap_2_2_Neighborhood::isNextFeasible(emili::Solution* base)
{
  std::vector<int> used_capacities = ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int orig_used_capacity = 0;
  int dest_used_capacity = 0;
  std::vector<int> demands = pis.getDemands();
  
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  if (orig_vehicle == dest_vehicle) return(false);
  if (orig_index >= (solution[orig_vehicle].size()-1)) return(false);
  if (dest_index >= (solution[dest_vehicle].size()-1)) return(false);
  
  int orig_customer1 = solution[orig_vehicle][orig_index];
  int orig_customer2 = solution[orig_vehicle][orig_index+1];
  int dest_customer1 = solution[dest_vehicle][dest_index];
  int dest_customer2 = solution[dest_vehicle][dest_index+1];
  
  orig_used_capacity = used_capacities[orig_vehicle] + demands[dest_customer1] + demands[dest_customer2] - demands[orig_customer1] - demands[orig_customer2];
  dest_used_capacity = used_capacities[dest_vehicle] + demands[orig_customer1] + demands[orig_customer2] - demands[dest_customer1] - demands[dest_customer2];
  
  if(orig_used_capacity > pis.getVehicleCapacity() || dest_used_capacity > pis.getVehicleCapacity()) {
    return false;
  }
  return true;
}

void emili::vrp::Swap_2_2_Neighborhood::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int orig_customer1 = solution[orig_vehicle][orig_index];
  int orig_customer2 = solution[orig_vehicle][orig_index+1];
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int dest_customer1 = solution[dest_vehicle][dest_index];
  int dest_customer2 = solution[dest_vehicle][dest_index+1];

  // change customers in origin
  solution[orig_vehicle][orig_index] = dest_customer1;
  solution[orig_vehicle][orig_index+1] = dest_customer2;
  used_capacity[orig_vehicle] = used_capacity[orig_vehicle] - demands[orig_customer1] - demands[orig_customer2] + demands[dest_customer1] + demands[dest_customer2];

  // change customer in destination
  solution[dest_vehicle][dest_index] = orig_customer1;
  solution[dest_vehicle][dest_index+1] = orig_customer2;
  used_capacity[dest_vehicle] = used_capacity[dest_vehicle] + demands[orig_customer1] + demands[orig_customer2] - demands[dest_customer1] - demands[dest_customer2];

  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
}

double emili::vrp::Swap_2_2_Neighborhood::calculateDeltaSolutionValue(emili::Solution* base)
{
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  int orig_customer1 = solution[orig_vehicle][orig_index];
  int orig_customer2 = solution[orig_vehicle][orig_index+1];
  int dest_customer1 = solution[dest_vehicle][dest_index];
  int dest_customer2 = solution[dest_vehicle][dest_index+1];
  
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  
  //inter-route only
  if (orig_vehicle == dest_vehicle) 
    return(solution_value);
  
  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  // origin distances
  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[orig_customer1];
    delta = delta +  dis_depot[dest_customer1];
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][orig_customer1];
    delta = delta + dis_matrix[orig_prev_customer][dest_customer1];
  }
  if (orig_post_customer == -1) {
    delta = delta - dis_depot[orig_customer2];
    delta = delta + dis_depot[dest_customer2];
  } else {
    delta = delta - dis_matrix[orig_customer2][orig_post_customer];
    delta = delta + dis_matrix[dest_customer2][orig_post_customer];
  }
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances
  if (dest_prev_customer == -1) {
    delta = delta - dis_depot[dest_customer1];
    delta = delta + dis_depot[orig_customer1];
  } else {
    delta = delta - dis_matrix[dest_prev_customer][dest_customer1];
    delta = delta + dis_matrix[dest_prev_customer][orig_customer1];
  }
  
  if (dest_post_customer == -1) {
    delta = delta - dis_depot[dest_customer2];
    delta = delta + dis_depot[orig_customer2];
  } else {
    delta = delta - dis_matrix[dest_customer2][dest_post_customer];
    delta = delta + dis_matrix[orig_customer2][dest_post_customer];
  }
  
  //std::cout << "fdelta " << solution_value << "->"<< delta << std::endl;
  
  new_value = solution_value + delta;
  
  return new_value;
}

/*****************************************Swap_2_2_Neighborhood_With_Reparation**********************************/

void emili::vrp::Swap_2_2_Neighborhood_With_Reparation::calculateNeighborhoodSize(emili::Solution* base) 
{
  int customers = pis.getNcustomers();
  n_size = customers*customers;
}

void emili::vrp::Swap_2_2_Neighborhood_With_Reparation::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  //Set previous and posterior costuments
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[orig_vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[orig_vehicle].size()-2))
    orig_post_customer = solution[orig_vehicle][orig_index + 2];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[dest_vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_index < (solution[dest_vehicle].size()-2))
    dest_post_customer = solution[dest_vehicle][dest_index + 2];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}

emili::Solution* emili::vrp::Swap_2_2_Neighborhood_With_Reparation::random(Solution* currentSolution) 
{
  int orig_vehicle;
  int orig_index;
  int dest_vehicle;
  int dest_index;
  int n_checked;
  int total_checked = 0;
  
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();
  
  calculateNeighborhoodSize(new_solution);

  //Get random customer indexes to start
  /*orig_vehicle = emili::generateRandomNumber() % solution.size();
  orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
  do {
    dest_vehicle = emili::generateRandomNumber() %  solution.size();
  } while (dest_vehicle==orig_vehicle);
  dest_index = emili::generateRandomNumber() % solution[dest_vehicle].size();
  next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));*/
  
  //Cycle indexes
  /*n_checked = total_checked = 1;  
  while (!isNextFeasible(currentSolution)) {
    if (total_checked >= size()) 
      return(new_solution);
    if (n_checked == (n_customers-1)) {
      // move origin clients once check all destination
      orig_index++;
      if (orig_index == solution[orig_vehicle].size()) {
         orig_index = 0;
         orig_vehicle++;
         if (orig_vehicle == solution.size())
            orig_vehicle = 0;

      }
      n_checked = 0;
    } else {
      //move all destination clients first
      dest_index++;
      if (dest_index == solution[dest_vehicle].size()) {
         //rolling of the neighbors
         dest_vehicle++;
         dest_index = 0;
         if (dest_vehicle == solution.size())
            dest_vehicle=0;
      }
    }
    next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    n_checked++;
    total_checked++;
  }*/
  do{
    orig_vehicle = emili::generateRandomNumber() % solution.size();
    orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
    do {
      dest_vehicle = emili::generateRandomNumber() %  solution.size();
    } while (dest_vehicle==orig_vehicle);
    dest_index = emili::generateRandomNumber() % solution[dest_vehicle].size();
    next_move = std::make_pair( std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
  }while( ((solution[orig_vehicle].size() - 1 - orig_index) < 2) || ((solution[dest_vehicle].size() - 1 - dest_index) < 2) );
  
  std::cout << "random swap22 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << std::endl; 
  std::cout << solution[orig_vehicle].size() - 1 - orig_index << std::endl;
  std::cout << solution[dest_vehicle].size() - 1 - dest_index  << std::endl;
  //std::cout << dest_vehicle << std::endl;
  //std::cout << dest_index << std::endl;
  //std::cout << solution[dest_vehicle][0] << std::endl;

  setAuxiliaryVariables(solution);
  makeMove(new_solution);
  repair(new_solution);
  return new_solution;
}

void emili::vrp::Swap_2_2_Neighborhood_With_Reparation::setNextNeighborPos(emili::Solution* base)
{
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int current_customer = solution[orig_vehicle][orig_index];
  
  do {
    if (!isFirstTime) {
      if (c_iterations > n_customers) {
        //finished moves of a costumer
        orig_index++;
        if (orig_index == solution[orig_vehicle].size()){
          orig_index = 0;
          orig_vehicle++;
          if (orig_vehicle == solution.size())
            orig_vehicle = 0;
        }
	/*if(orig_vehicle == dest_vehicle && (dest_vehicle != solution.size())){
		dest_vehicle++;
		dest_index = 0;
	}*/
        c_iterations = 1;
      } else {
        dest_index++;
        if (dest_index == solution[dest_vehicle].size()) {
          //rolling of the neighbors
          dest_vehicle++;
          dest_index = 0;
          if (dest_vehicle == solution.size())
            dest_vehicle=0;
        }
      }
    } else {
      isFirstTime = false;
    }
    
    //std::cout << "swap22 o:" << orig_vehicle << "," << orig_index << " d:" << dest_vehicle << "," << dest_index << 
                // " step: " << c_iterations << "/" << n_customers << "  " << n_iterations << "/" << n_size << std::endl;
    
    next_move = std::make_pair(std::make_pair(orig_vehicle, orig_index), std::make_pair(dest_vehicle, dest_index));
    setAuxiliaryVariables(solution);
    n_iterations++;
    c_iterations++;
    
  } while (dest_vehicle==orig_vehicle);
}

bool emili::vrp::Swap_2_2_Neighborhood_With_Reparation::isNextFeasible(emili::Solution* base)
{
  std::vector<int> used_capacities = ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int orig_used_capacity = 0;
  int dest_used_capacity = 0;
  std::vector<int> demands = pis.getDemands();
  
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  if (orig_vehicle == dest_vehicle) return(false);
  if (orig_index >= (solution[orig_vehicle].size()-1)) return(false);
  if (dest_index >= (solution[dest_vehicle].size()-1)) return(false);
  
  int orig_customer1 = solution[orig_vehicle][orig_index];
  int orig_customer2 = solution[orig_vehicle][orig_index+1];
  int dest_customer1 = solution[dest_vehicle][dest_index];
  int dest_customer2 = solution[dest_vehicle][dest_index+1];
  
  orig_used_capacity = used_capacities[orig_vehicle] + demands[dest_customer1] + demands[dest_customer2] - demands[orig_customer1] - demands[orig_customer2];
  dest_used_capacity = used_capacities[dest_vehicle] + demands[orig_customer1] + demands[orig_customer2] - demands[dest_customer1] - demands[dest_customer2];
  
  if(orig_used_capacity > pis.getVehicleCapacity() || dest_used_capacity > pis.getVehicleCapacity()) {
    return false;
  }
  return true;
}

void emili::vrp::Swap_2_2_Neighborhood_With_Reparation::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int orig_customer1 = solution[orig_vehicle][orig_index];
  int orig_customer2 = solution[orig_vehicle][orig_index+1];
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  int dest_customer1 = solution[dest_vehicle][dest_index];
  int dest_customer2 = solution[dest_vehicle][dest_index+1];

  // change customers in origin
  solution[orig_vehicle][orig_index] = dest_customer1;
  solution[orig_vehicle][orig_index+1] = dest_customer2;
  used_capacity[orig_vehicle] = used_capacity[orig_vehicle] - demands[orig_customer1] - demands[orig_customer2] + demands[dest_customer1] + demands[dest_customer2];

  // change customer in destination
  solution[dest_vehicle][dest_index] = orig_customer1;
  solution[dest_vehicle][dest_index+1] = orig_customer2;
  used_capacity[dest_vehicle] = used_capacity[dest_vehicle] + demands[orig_customer1] + demands[orig_customer2] - demands[dest_customer1] - demands[dest_customer2];

  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
}

double emili::vrp::Swap_2_2_Neighborhood_With_Reparation::calculateDeltaSolutionValue(emili::Solution* base)
{
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second;
  int dest_vehicle = next_move.second.first;
  int dest_index = next_move.second.second;
  
  int orig_customer1 = solution[orig_vehicle][orig_index];
  int orig_customer2 = solution[orig_vehicle][orig_index+1];
  int dest_customer1 = solution[dest_vehicle][dest_index];
  int dest_customer2 = solution[dest_vehicle][dest_index+1];
  
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  
  //inter-route only
  if (orig_vehicle == dest_vehicle) 
    return(solution_value);
  
  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  // origin distances
  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[orig_customer1];
    delta = delta +  dis_depot[dest_customer1];
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][orig_customer1];
    delta = delta + dis_matrix[orig_prev_customer][dest_customer1];
  }
  if (orig_post_customer == -1) {
    delta = delta - dis_depot[orig_customer2];
    delta = delta + dis_depot[dest_customer2];
  } else {
    delta = delta - dis_matrix[orig_customer2][orig_post_customer];
    delta = delta + dis_matrix[dest_customer2][orig_post_customer];
  }
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances
  if (dest_prev_customer == -1) {
    delta = delta - dis_depot[dest_customer1];
    delta = delta + dis_depot[orig_customer1];
  } else {
    delta = delta - dis_matrix[dest_prev_customer][dest_customer1];
    delta = delta + dis_matrix[dest_prev_customer][orig_customer1];
  }
  
  if (dest_post_customer == -1) {
    delta = delta - dis_depot[dest_customer2];
    delta = delta + dis_depot[orig_customer2];
  } else {
    delta = delta - dis_matrix[dest_customer2][dest_post_customer];
    delta = delta + dis_matrix[orig_customer2][dest_post_customer];
  }
  
  //std::cout << "fdelta " << solution_value << "->"<< delta << std::endl;
  
  new_value = solution_value + delta;
  
  return new_value;
}

void emili::vrp::Swap_2_2_Neighborhood_With_Reparation::repair(emili::Solution* base)
{
// esto va despues de make_move
  std::vector< std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector<int> demands = pis.getDemands();
  std::vector<std::vector<double>> DistanceMatrix = pis.getDistanceMatrix();
  std::vector<double> WarehousesDistance = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  int vehicle_capacity = pis.getVehicleCapacity();

  std::vector<int> temporal_solution;
  int temporal_customer = 0;
  double temporal_solution_value  = 0;
  double temporal_demand = 0;
  int empty;
  int random_change;
  for(int i = 0; i < solution.size(); i++){
    while(used_capacity[i] > vehicle_capacity){

      random_change = emili::generateRandomNumber() % solution[i].size();
      temporal_customer = solution[i][random_change];

      if(temporal_demand + demands[temporal_customer] > vehicle_capacity){
        solution.push_back(temporal_solution);
        used_capacity.push_back(temporal_demand);
        solution_value += temporal_solution_value;

        temporal_solution.clear();
        temporal_demand = 0;
        temporal_solution_value = 0;
      }
      /*if(random_change == 0){
        solution_value += - WarehousesDistance[temporal_customer] + WarehousesDistance[solution[i][1]];
        solution_value += - DistanceMatrix[solution[i][0]][solution[i][1]];
      }
      else if(random_change == solution[i].size() - 1){
        solution_value = solution_value - WarehousesDistance[temporal_customer];
        solution_value = solution_value + WarehousesDistance[solution[i][random_change -1]];
      }
      else{
        solution_value += - DistanceMatrix[solution[i][random_change] ][ solution[i][random_change - 1] ];
        solution_value += - DistanceMatrix[solution[i][random_change] ][ solution[i][random_change + 1] ];
        solution_value += - DistanceMatrix[solution[i][random_change - 1] ][ solution[i][random_change + 1] ];
      }*/
      used_capacity[i] = used_capacity[i] - demands[temporal_customer];
      solution[i].erase(solution[i].begin() + random_change);
      temporal_demand += demands[temporal_customer];
      temporal_solution.push_back(temporal_customer);

      /*if(temporal_solution.size() == 0){
        temporal_solution.push_back(temporal_customer);
        temporal_solution_value += 2*WarehousesDistance[temporal_customer];
      }
      else{
        temporal_solution.push_back(temporal_customer);
        temporal_solution_value = temporal_solution_value - WarehousesDistance[temporal_solution[temporal_solution.size() - 2]];
        temporal_solution_value = temporal_solution_value + WarehousesDistance[temporal_solution[temporal_solution.size() - 1]];
        temporal_solution_value = temporal_solution_value + DistanceMatrix[temporal_solution[temporal_solution.size() - 2]][temporal_solution[temporal_solution.size() - 1]];
      }*/

    }
  }

  solution.push_back(temporal_solution);
  used_capacity.push_back(temporal_demand);
  solution_value += temporal_solution_value;

  int i = 0;
  while(i < solution.size() ){
    
    
    if(solution[i].size() == 0){
      solution.erase(solution.begin() + i);
      used_capacity.erase(used_capacity.begin() + i);
      i = 0;
    }
    i++;
  }

  double fitness = 0;
  for(int i = 0; i < solution.size(); i++){
    for(int j = 0; j < solution[i].size();j++){
      if(j == 0 || j == solution.size() - 1){
        fitness += WarehousesDistance[solution[i][j]];
      }
      else{
        fitness += DistanceMatrix[j][j+1];
      }
    }
  }


  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(fitness);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
  //std::cout << "fitness " << solution_value << std::endl;


  //std::cout << "final" << std::endl;
}

/***********************CROSS-EXCHANGE****************************/

void emili::vrp::Cross_Exchange_Neighborhood::calculateNeighborhoodSize(emili::Solution* base) 
{ 
  //Shift neighborhood considers all possible positions to move
  //moves must filtered based on the move
  //int n_vehicles = ((emili::vrp::VrpSolution*) base)->getNVehicles();
  n_size = this->n_customers*this->n_customers; 
};


emili::Solution* emili::vrp::Cross_Exchange_Neighborhood::random(Solution* currentSolution) 
{
  int orig_vehicle;
  int orig_index;
  int orig_index2;

  int dest_vehicle;
  int dest_index;
  int dest_index2;
  int count_condition = 0;
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();



  if(solution.size() >= 2){
  	for(int i = 0; i < solution.size(); i++){
  		if(solution[i].size() >= 3){
			count_condition++;
		}
  	}
	if(count_condition >= 2){
		    do{
		    	do{
                        	orig_vehicle = emili::generateRandomNumber() % solution.size();
                        	dest_vehicle = emili::generateRandomNumber() % solution.size();
			}while(orig_vehicle == dest_vehicle);

			//do{
			orig_index = emili::generateRandomNumber() % solution[orig_vehicle].size();
			orig_index2 = emili::generateRandomNumber() % solution[orig_vehicle].size();
			//}while(orig_index+2 <= orig_index2);

                        //do{
                        dest_index = emili::generateRandomNumber() % solution[dest_vehicle].size();
                        dest_index2 = emili::generateRandomNumber() % solution[dest_vehicle].size();
                        //}while(dest_index+2 <= dest_index2);
			next_move = std::make_pair( std::make_pair(orig_vehicle, std::make_pair(orig_index,orig_index2) )    , std::make_pair(dest_vehicle, std::make_pair(dest_index,dest_index2) ) );
		    }while(!isNextFeasible(new_solution));
	}
	setAuxiliaryVariables(solution);
	makeMove(new_solution);
  }
  //new_solution->getSolutionRepresentation();
  std::cout <<"test de cross "<< new_solution -> getSolutionRepresentation()<<std::endl;
  return new_solution;
}

bool emili::vrp::Cross_Exchange_Neighborhood::isNextFeasible(emili::Solution* base){
  std::vector<int> used_capacities = ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  int new_used_capacity = 0;
  std::vector<int> demands = pis.getDemands();

  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second.first;
  int orig_index2 = next_move.first.second.second;

  int dest_vehicle =  next_move.second.first;
  int dest_index = next_move.second.second.first;
  int dest_index2 = next_move.second.second.second;

  int capacity_orig = 0;
  int capacity_dest = 0;

  if(( orig_index+2 <= orig_index2 ) && (dest_index+2 <= dest_index2) ){
   	for(int i = orig_index+1 ; i < orig_index2;i++){
		capacity_orig += demands[solution[orig_vehicle][i]];
	}
        for(int i = dest_index+1 ; i < dest_index2;i++){
                capacity_dest += demands[solution[dest_vehicle][i]];
        }
	if(used_capacities[orig_vehicle] - capacity_orig + capacity_dest > pis.getVehicleCapacity()){
		return false;
	}
	if(used_capacities[dest_vehicle] - capacity_dest + capacity_orig > pis.getVehicleCapacity()){
		return false;
	}
	if(dest_index2 == solution[dest_vehicle].size()-1){
		return false;
	}
	if(orig_index2 == solution[orig_vehicle].size()-1){
		return false;
	}
	return true;
  }
  return false;

}


void emili::vrp::Cross_Exchange_Neighborhood::setAuxiliaryVariables(std::vector < std::vector<int> > & solution){

  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second.first;
  int orig_index2 = next_move.first.second.second;

  int dest_vehicle =  next_move.second.first;
  int dest_index = next_move.second.second.first;
  int dest_index2 = next_move.second.second.second;

  pre_post_customers = std::make_pair( std::make_pair(orig_vehicle, std::make_pair(orig_index,orig_index2) ) , std::make_pair(dest_vehicle, std::make_pair(dest_index,dest_index2) ) );

}



void emili::vrp::Cross_Exchange_Neighborhood::makeMove(emili::Solution* base){

  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  double solution_value = calculateDeltaSolutionValue(base);
  //double solution_value = 1;
  std::vector<int> demands = pis.getDemands();
  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second.first;
  int orig_index2 = next_move.first.second.second;

  int dest_vehicle =  next_move.second.first;
  int dest_index = next_move.second.second.first;
  int dest_index2 = next_move.second.second.second;
  
  int capacity_orig = 0;
  int capacity_dest = 0;
  std::vector <int> aux1(solution[orig_vehicle].begin() + orig_index+1 , solution[orig_vehicle].begin() + orig_index2);
  std::vector<int> aux2(solution[dest_vehicle].begin() + dest_index+1 , solution[dest_vehicle].begin() + dest_index2);
  
  std::vector<int> aux3;
  std::vector<int> aux4;

  for(int i = 0; i < solution[orig_vehicle].size(); i++){
   	if(i <= orig_index || i >= orig_index2){
		aux3.push_back(solution[orig_vehicle][i]);
        }
	else{
		aux3.push_back(0);
	}
  }

  for(int i = 0; i < solution[dest_vehicle].size(); i++){
        if(i <= dest_index || i >= dest_index2){
                aux4.push_back(solution[dest_vehicle][i]);
        }
        else{
                aux4.push_back(0);
        }
  }


  std::copy(aux2.begin(), aux2.end(), aux3.begin()+orig_index);
  std::copy(aux1.begin(), aux1.end(), aux4.begin()+dest_index);
  
  solution[orig_vehicle] = aux3;
  solution[dest_vehicle] = aux4;
  //std::cout << "aqui es 1" << std::endl;

  for(int i = orig_index+1 ; i < orig_index2;i++){
                capacity_orig += demands[solution[orig_vehicle][i]];
  }

  for(int i = dest_index+1 ; i < dest_index2;i++){
                capacity_dest += demands[solution[dest_vehicle][i]];
  }

  used_capacity[orig_vehicle] = used_capacity[orig_vehicle] - capacity_orig + capacity_dest;
  used_capacity[dest_vehicle] = used_capacity[dest_vehicle] - capacity_dest + capacity_orig;
  //std::cout << "aqui es 2" << std::endl;
}

double emili::vrp::Cross_Exchange_Neighborhood::calculateDeltaSolutionValue(emili::Solution* base){

  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;

  int orig_vehicle = next_move.first.first;
  int orig_index = next_move.first.second.first;
  int orig_index2 = next_move.first.second.second;

  int dest_vehicle =  next_move.second.first;
  int dest_index = next_move.second.second.first;
  int dest_index2 = next_move.second.second.second;
  std::cout <<"aqui1"<<std::endl;
  //primer arco en el vehiculo origen
  solution_value = solution_value - dis_matrix[solution[orig_vehicle][orig_index]][solution[orig_vehicle][orig_index+1]] + dis_matrix[solution[orig_vehicle][orig_index]][solution[dest_vehicle][dest_index+1]];
  std::cout <<"aqui2"<<std::endl;
  // primer arco en el vehiculo destino
  solution_value = solution_value - dis_matrix[solution[dest_vehicle][dest_index]][solution[dest_vehicle][dest_index+1]] + dis_matrix[solution[dest_vehicle][dest_index]][solution[orig_vehicle][orig_index+1]];
  std::cout <<"aqui3"<<std::endl;
  //
  solution_value = solution_value - dis_matrix[solution[orig_vehicle][orig_index2]][solution[orig_vehicle][orig_index2-1]] + dis_matrix[solution[orig_vehicle][orig_index2]][solution[dest_vehicle][dest_index2-1]];
  std::cout <<"aqui4"<<std::endl;
  //
  solution_value = solution_value - dis_matrix[solution[dest_vehicle][dest_index2]][solution[dest_vehicle][dest_index2-1]] + dis_matrix[solution[dest_vehicle][dest_index2]][solution[orig_vehicle][orig_index-1]];
  std::cout <<"aqui5"<<std::endl;
  
  std::cout << solution_value << std::endl;
  return solution_value;
}

void emili::vrp::Cross_Exchange_Neighborhood::setNextNeighborPos(emili::Solution* base){
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
}




/*-------------------------------------------------INTRA-ROUTE----------------------------------------------------*/

emili::Neighborhood::NeighborhoodIterator emili::vrp::IntraRoute_Neighborhood::begin(emili::Solution* base)
{
  emili::vrp::VrpSolution* solution = ((emili::vrp::VrpSolution*) base);
  calculateNeighborhoodSize(base);
  n_iterations = 1;
  last_n_iterations = 1;
  last_n_size = n_size;
  isFirstTime = true;
  return emili::Neighborhood::NeighborhoodIterator(this,base);
}

void emili::vrp::IntraRoute_Neighborhood::reset()
{
  //start position
  next_move = std::make_pair(0,std::make_pair(0,0));
  last_move = std::make_pair(0,std::make_pair(0,0));
  pre_post_customers = std::make_pair(std::make_pair(-1,-1),std::make_pair(-1,-1));
  
  n_iterations = 1;
  last_n_iterations = 1;
  isFirstTime = true;
}

emili::Solution* emili::vrp::IntraRoute_Neighborhood::computeStep(emili::Solution* step)  
{
  emili::iteration_increment();
  double solution_value = ((emili::vrp::VrpSolution*) step)->getSolutionValue();
  
  //std::cout << "current solution "<< step->getSolutionRepresentation() << " " << step->getSolutionValue() << std::endl;
  
  while(n_iterations <= n_size) {
    setNextNeighborPos(step);
    if (isNextFeasible(step) ){
      // Modify actual Solution for the new one
      makeMove(step);
      last_n_iterations = n_iterations;
      last_r_iterations = r_iterations;
      last_n_size = n_size;
      
      //Update neighborhood
      calculateNeighborhoodSize(step);
      n_iterations = 1;
      r_iterations = 1;
      last_move = next_move;
      //std::cout << "feasible" << " " << step->getSolutionRepresentation() << step->getSolutionValue() << std::endl;
      //getchar();
      return step;
    }  //else std::cout << "infeasible" << " keep " << step->getSolutionRepresentation() << step->getSolutionValue() << std::endl;
  }
  
  return nullptr;
}

void emili::vrp::IntraRoute_Neighborhood::reverseLastMove(Solution* step)
{
  ((emili::vrp::VrpSolution*) step)->reverse();
  next_move = last_move;
  n_iterations = last_n_iterations;
  n_size = last_n_size;
}

/*********************************** Reinsetion_Neighborhood **********************************/

void emili::vrp::Reinsertion_Neighborhood::calculateNeighborhoodSize(emili::Solution* base) 
{
  emili::vrp::VrpSolution* solution = ((emili::vrp::VrpSolution*) base);
  int customers = pis.getNcustomers();
  int n_vehicles = solution->getNVehicles();
  n_size = 0;
  for (int i=0; i<n_vehicles; i++) {
    //n_size = n_size + (solution->getSolution()[i].size()-2) * (solution->getSolution()[i].size());
    n_size = n_size + (solution->getSolution()[i].size()* (solution->getSolution()[i].size()-1));
  }
}

void emili::vrp::Reinsertion_Neighborhood::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  int vehicle = next_move.first;
  int orig_index = next_move.second.first;
  int dest_index = next_move.second.second;
  
  //Set previous and posterior costuments
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[vehicle].size()-1))
    orig_post_customer = solution[vehicle][orig_index + 1];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_index < solution[vehicle].size())
    dest_post_customer = solution[vehicle][dest_index];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}

emili::Solution* emili::vrp::Reinsertion_Neighborhood::random(Solution* currentSolution) 
{
  int vehicle;
  int orig_index;
  int dest_index;
  
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();
  
  //Get random customer index
  do {
    do {
      vehicle = emili::generateRandomNumber() % solution.size();
      orig_index = emili::generateRandomNumber() % solution[vehicle].size();
      dest_index = emili::generateRandomNumber() %  solution[vehicle].size();
      next_move = std::make_pair(vehicle, std::make_pair(orig_index, dest_index));
    } while (dest_index==orig_index || orig_index == (dest_index-1));
  } while (!isNextFeasible(new_solution));
  
  //std::cout << "random reinsertion v:" << vehicle << " o:" << orig_index << " d:" << dest_index << std::endl; 
  setAuxiliaryVariables(solution);
  makeMove(new_solution);

  return new_solution;
}

void emili::vrp::Reinsertion_Neighborhood::setNextNeighborPos(emili::Solution* base)
{
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  
  int vehicle = next_move.first;
  int dest_index = next_move.second.second;
  int orig_index = next_move.second.first;

  do {
    if (!isFirstTime) {
      dest_index++;
      if (dest_index == (solution[vehicle].size()+1)) {
        orig_index++;
        dest_index = 0;
        if (orig_index == solution[vehicle].size()) {
          orig_index = 0;
          vehicle++;
          if (vehicle == solution.size()) vehicle = 0;
        }
      }
      
    } else {
      isFirstTime = false;
    }
  } while (dest_index==orig_index || orig_index == (dest_index-1));
  
  //std::cout << "reinsertion v:" << vehicle << " o:" << orig_index << " d:" << dest_index << " step: " << n_iterations << "/" << n_size<< std::endl;
  n_iterations++;
  
  next_move = std::make_pair(vehicle, std::make_pair(orig_index, dest_index));
  setAuxiliaryVariables(solution);
  
}

bool emili::vrp::Reinsertion_Neighborhood::isNextFeasible(emili::Solution* base)
{
  return true;
}

void emili::vrp::Reinsertion_Neighborhood::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  
  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();
  int vehicle = next_move.first;
  int orig_index = next_move.second.first;
  int dest_index = next_move.second.second;
  int customer = solution[vehicle][orig_index];

  //remove
  solution[vehicle].emplace((solution[vehicle].begin() + dest_index), customer);
  if (orig_index > dest_index) {
    solution[vehicle].erase(solution[vehicle].begin() + orig_index+1);
  } else {
    solution[vehicle].erase(solution[vehicle].begin() + orig_index);
  }
  
  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
}

double emili::vrp::Reinsertion_Neighborhood::calculateDeltaSolutionValue(emili::Solution* base)
{
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  
  int vehicle = next_move.first;
  int orig_index = next_move.second.first;
  int dest_index = next_move.second.second;
  int customer = solution[vehicle][orig_index];
  
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  
  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  // origin distances
  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[customer];
    if (orig_post_customer != -1) {
      delta = delta + dis_depot[orig_post_customer];
      delta = delta - dis_matrix[customer][orig_post_customer];
    }
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][customer];
    if (orig_post_customer == -1) {
      delta = delta - dis_depot[customer];
      delta = delta + dis_depot[orig_prev_customer];
    } else {
      delta = delta - dis_matrix[customer][orig_post_customer];
      delta = delta + dis_matrix[orig_prev_customer][orig_post_customer];
    }
  }
  
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances
  if (dest_prev_customer == -1) {
    delta = delta + dis_depot[customer];
    if (dest_post_customer == -1) {
      delta = delta + dis_depot[customer];
    } else {
      delta = delta - dis_depot[dest_post_customer];
      delta = delta + dis_matrix[customer][dest_post_customer];
    }
  } else {
    delta = delta + dis_matrix[dest_prev_customer][customer];
    if (dest_post_customer == -1) {
      delta = delta - dis_depot[dest_prev_customer];
      delta = delta + dis_depot[customer];
    } else {
      delta = delta + dis_matrix[customer][dest_post_customer];
      delta = delta - dis_matrix[dest_prev_customer][dest_post_customer];
    }
  }
  
  //std::cout << "fdelta " << solution_value << "->"<< delta << std::endl;
  new_value = solution_value + delta;
  
  return new_value;
}

/****************************************Exchange_Neighborhood******************************************/

void emili::vrp::Exchange_Neighborhood::calculateNeighborhoodSize(emili::Solution* base) 
{
  emili::vrp::VrpSolution* solution = ((emili::vrp::VrpSolution*) base);
  int n_vehicles = solution->getNVehicles();
  n_size = 0;
  for (int i=0; i<n_vehicles; i++) {
    for (int j=1; j<solution->getSolution()[i].size(); j++) {
      n_size = n_size + j;
    }
  }
}

void emili::vrp::Exchange_Neighborhood::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  int vehicle = next_move.first;
  int orig_index = next_move.second.first;
  int dest_index = next_move.second.second;
  
  //Set previous and posterior costuments
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[vehicle].size()-1))
    orig_post_customer = solution[vehicle][orig_index + 1];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_index < (solution[vehicle].size()-1))
    dest_post_customer = solution[vehicle][dest_index + 1];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}

emili::Solution* emili::vrp::Exchange_Neighborhood::random(Solution* currentSolution) 
{
  int vehicle;
  int orig_index;
  int dest_index;
  
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();
  
  //Get random customer index
  do {
    do {
      vehicle = emili::generateRandomNumber() % solution.size();
      orig_index = emili::generateRandomNumber() % solution[vehicle].size();
      dest_index = emili::generateRandomNumber() %  solution[vehicle].size();
      next_move = std::make_pair(vehicle, std::make_pair(orig_index, dest_index));
    } while (dest_index < orig_index || orig_index == dest_index);;
  } while (!isNextFeasible(new_solution));
  

  //std::cout << "random exchange v:" << vehicle << " o:" << orig_index << " d:" << dest_index << std::endl; 
  //std::cout << "holi 1 "<<std::endl;
  setAuxiliaryVariables(solution);
  makeMove(new_solution);
  
  return new_solution;
}

void emili::vrp::Exchange_Neighborhood::setNextNeighborPos(emili::Solution* base)
{
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  
  int vehicle = next_move.first;
  int dest_index = next_move.second.second;
  int orig_index = next_move.second.first;
  
  do {
    if (!isFirstTime) {
      dest_index++;
      if (dest_index >= solution[vehicle].size()) {
        orig_index++;
        dest_index = orig_index + 1;
        if (dest_index >= solution[vehicle].size()) {
          orig_index=0;
          dest_index=1;
          do {
            vehicle++;
            if (vehicle >= solution.size()) 
              vehicle = 0;
          } while (dest_index >= solution[vehicle].size());
        }
      }
    } else {
      isFirstTime = false;
    }
    
    //std::cout << "exchange v:" << vehicle << " o:" << orig_index << " d:" << dest_index << " step: " << n_iterations << "/" << n_size << std::endl;
    
    next_move = std::make_pair(vehicle, std::make_pair(orig_index, dest_index));
    setAuxiliaryVariables(solution);
    n_iterations++;
  } while (dest_index==orig_index);
  
}

bool emili::vrp::Exchange_Neighborhood::isNextFeasible(emili::Solution* base)
{
  return true;
}

void emili::vrp::Exchange_Neighborhood::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  
  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();
  
  int vehicle = next_move.first;
  int orig_index = next_move.second.first;
  int dest_index = next_move.second.second;
  int customer = solution[vehicle][orig_index];
  int aux;
  
  //swap
  aux = solution[vehicle][orig_index];
  solution[vehicle][orig_index] = solution[vehicle][dest_index];
  solution[vehicle][dest_index] = aux;
  
  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
}

double emili::vrp::Exchange_Neighborhood::calculateDeltaSolutionValue(emili::Solution* base)
{
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  
  int vehicle = next_move.first;
  int orig_index = next_move.second.first;
  int dest_index = next_move.second.second;
  int customer1 = solution[vehicle][orig_index];
  int customer2 = solution[vehicle][dest_index];
  
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  
  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  // origin distances
  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[customer1];
    delta = delta + dis_depot[customer2];
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][customer1];
    delta = delta + dis_matrix[orig_prev_customer][customer2];
  }
  if (orig_post_customer == -1) {
    delta = delta - dis_depot[customer1];
    delta = delta + dis_depot[customer2];
  } else if (orig_post_customer != customer2) {
    delta = delta - dis_matrix[customer1][orig_post_customer];
    delta = delta + dis_matrix[customer2][orig_post_customer];
  }
  
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances
  if (dest_prev_customer == -1) {
    delta = delta - dis_depot[customer2];
    delta = delta + dis_depot[customer1];
  } else if (dest_prev_customer != customer1) {
    delta = delta - dis_matrix[dest_prev_customer][customer2];
    delta = delta + dis_matrix[dest_prev_customer][customer1];
  }
  if (dest_post_customer == -1) {
    delta = delta - dis_depot[customer2];
    delta = delta + dis_depot[customer1];
  } else {
    delta = delta - dis_matrix[customer2][dest_post_customer];
    delta = delta + dis_matrix[customer1][dest_post_customer];
  }
  
  //std::cout << "fdelta " << solution_value << "->"<< delta << std::endl;
  
  new_value = solution_value + delta;
  
  return new_value;
}

/*************************************TwoOPT_Neighborhood************************************/

emili::Solution* emili::vrp::TwoOPT_Neighborhood::random(Solution* currentSolution) 
{
  int vehicle;
  int orig_index;
  int dest_index;
  int aux;
  
  std::vector< std::vector<int>> solution = ((emili::vrp::VrpSolution*) currentSolution)->getSolution(); 
  emili::Solution* new_solution = ((emili::vrp::VrpSolution*) currentSolution)->clone();
  
  //Get random customer index
  do{
    do {
      vehicle = emili::generateRandomNumber() % solution.size();
    } while (solution[vehicle].size() < 4);

    do{
      orig_index = emili::generateRandomNumber() % (solution[vehicle].size()-2);
      aux = emili::generateRandomNumber() % (solution[vehicle].size() - orig_index);
      dest_index = orig_index + aux;
    } while (dest_index==orig_index && orig_index > dest_index);
    next_move = std::make_pair(vehicle, std::make_pair(orig_index, dest_index));
  } while (!isNextFeasible(new_solution));
  
  //std::cout << "random 2opt v:" << vehicle << " o:" << orig_index << " d:" << dest_index << std::endl; 
  setAuxiliaryVariables(solution);
  makeMove(new_solution);
  
  return new_solution;
}

void emili::vrp::TwoOPT_Neighborhood::calculateNeighborhoodSize(emili::Solution* base) 
{
  emili::vrp::VrpSolution* solution = ((emili::vrp::VrpSolution*) base);
  int n_vehicles = solution->getNVehicles();
  int v_size;
  n_size = 1;
  for (int i=0; i<n_vehicles; i++) {
    v_size = solution->getSolution()[i].size();
    if (v_size > 3){
      for (int j=1; j<(v_size-1); j++) {
      n_size = n_size + j;
      }
    }
  }
}

void emili::vrp::TwoOPT_Neighborhood::setAuxiliaryVariables(std::vector < std::vector<int> > & solution)
{
  int vehicle = next_move.first;
  int orig_index = next_move.second.first;
  int dest_index = next_move.second.second;
  
  //Set previous and posterior costuments
  int orig_prev_customer = -1;
  if (orig_index > 0) 
    orig_prev_customer = solution[vehicle][orig_index - 1];
  
  int orig_post_customer = -1;
  if (orig_index < (solution[vehicle].size()-1))
    orig_post_customer = solution[vehicle][orig_index + 1];
  
  int dest_prev_customer = -1;
  if (dest_index > 0)
    dest_prev_customer = solution[vehicle][dest_index - 1];
  
  int dest_post_customer = -1;
  if (dest_index < (solution[vehicle].size()-1))
    dest_post_customer = solution[vehicle][dest_index + 1];
  
  pre_post_customers = std::make_pair(std::make_pair(orig_prev_customer, orig_post_customer), 
                                      std::make_pair(dest_prev_customer, dest_post_customer));
}

void emili::vrp::TwoOPT_Neighborhood::setNextNeighborPos(emili::Solution* base)
{
  std::vector<std::vector<int>> solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  
  int vehicle = next_move.first;
  int dest_index = next_move.second.second;
  int orig_index = next_move.second.first;
  
  do {
    if (!isFirstTime) {
      dest_index++;
      if (dest_index >= solution[vehicle].size()) {
        orig_index++;
        dest_index = orig_index + 2;
        if (dest_index >= solution[vehicle].size()) {
          orig_index=0;
          dest_index = 2;
          vehicle++;
          if (vehicle == solution.size())
            vehicle = 0;
        }
      }
    } else {
      isFirstTime = false;
    }
    
    //std::cout << "2opt v:" << vehicle << " o:" << orig_index << " d:" << dest_index << " step: " << n_iterations << std::endl;

    next_move = std::make_pair(vehicle, std::make_pair(orig_index, dest_index));
    setAuxiliaryVariables(solution);
  } while ((dest_index - orig_index) < 2 || solution[vehicle].size()<4);
  n_iterations++;
}

bool emili::vrp::TwoOPT_Neighborhood::isNextFeasible(emili::Solution* base)
{
  return true;
}

void emili::vrp::TwoOPT_Neighborhood::makeMove(emili::Solution* base)
{
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  
  double solution_value = calculateDeltaSolutionValue(base);
  std::vector<int> demands = pis.getDemands();
  int vehicle = next_move.first;
  int orig_index = next_move.second.first;
  int dest_index = next_move.second.second;
  int aux;

  //swap
  for (int i=0; i<=((dest_index-orig_index)/2); i++){
    if ((orig_index+i) != (dest_index-i)){
      aux = solution[vehicle][orig_index+i];
      solution[vehicle][orig_index+i] = solution[vehicle][dest_index-i];
      solution[vehicle][dest_index-i] = aux;
    }
  }
  // Update solution
  // Evaluation must be calculated before changing the solution
  ( (emili::vrp::VrpSolution*) base )->setSolutionValue(solution_value);
  ( (emili::vrp::VrpSolution*) base )->setSolution(solution);
  ( (emili::vrp::VrpSolution*) base )->setUsedCapacity(used_capacity);
}

double emili::vrp::TwoOPT_Neighborhood::calculateDeltaSolutionValue(emili::Solution* base)
{
  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  double solution_value = ((emili::vrp::VrpSolution*) base)->getSolutionValue();
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  double new_value = 0;
  double delta = 0;
  
  int vehicle = next_move.first;
  int orig_index = next_move.second.first;
  int dest_index = next_move.second.second;
  int customer1 = solution[vehicle][orig_index];
  int customer2 = solution[vehicle][dest_index];
  
  int orig_prev_customer = pre_post_customers.first.first;
  int orig_post_customer = pre_post_customers.first.second;
  int dest_prev_customer = pre_post_customers.second.first;
  int dest_post_customer = pre_post_customers.second.second;
  
  //std::cout << "p " << orig_prev_customer << " po " << orig_post_customer<< std::endl;
  // origin distances
  if (orig_prev_customer == -1) {
    delta = delta - dis_depot[customer1];
    delta = delta + dis_depot[customer2];
  } else { 
    delta = delta - dis_matrix[orig_prev_customer][customer1];
    delta = delta + dis_matrix[orig_prev_customer][customer2];
  }
  
  //std::cout << "delta " << delta << std::endl;
  
  //std::cout << "d " << dest_prev_customer << " do " << dest_post_customer<< std::endl;
  // remove destination distances
  if (dest_post_customer == -1) {
    delta = delta - dis_depot[customer2];
    delta = delta + dis_depot[customer1];
  } else {
    delta = delta - dis_matrix[customer2][dest_post_customer];
    delta = delta + dis_matrix[customer1][dest_post_customer];
  }
  
  //std::cout << "fdelta " << solution_value << "->"<< delta << std::endl;
  
  new_value = solution_value + delta;
  
  return new_value;
}

/*-------------------------------------------------------------------------------------------------------*
 *----------------------------------------------------REMOVAL--------------------------------------------*
 *-------------------------------------------------------------------------------------------------------*/

void emili::vrp::Random_Removal::removalArcs(emili::Solution* base){
  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  double fitness = ((emili::vrp::VrpSolution*) base)->getSolutionValue();

  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  std::vector<int> demands = pis.getDemands();
  // pis.getVehicleCapacity()

  int random_vehicle;
  int random_client;

  for(int i = 0; i < arcs; i++){
    random_vehicle = emili::generateRandomNumber() % solution.size();

    random_client = emili::generateRandomNumber() % solution[random_vehicle].size();

    if(random_client == 0){
      fitness = fitness - dis_depot[solution[random_vehicle][random_client]] + dis_depot[solution[random_vehicle][random_client + 1]]; 
    }

    if(random_client == solution[random_vehicle].size() - 1){
      fitness = fitness - dis_depot[solution[random_vehicle][random_client]] + dis_depot[solution[random_vehicle][random_client - 1]]; 
    }

    if(random_client != 0){
      fitness = fitness - dis_matrix [solution[random_vehicle][random_client] ][solution[random_vehicle][random_client - 1] ]; 
    }

    if(random_client != solution[random_vehicle].size() - 1){
      fitness = fitness - dis_matrix [solution[random_vehicle][random_client] ][solution[random_vehicle][random_client + 1] ]; 
    }

    used_capacity[random_vehicle] = used_capacity[random_vehicle] - demands[solution[random_vehicle][random_client]];
    clients.push_back(solution[random_vehicle][random_client]);
    solution[random_vehicle].erase(solution[random_vehicle].begin() + random_client);


    //((emili::vrp::VrpSolution*) base)->setSolutionValue(fitness);
    ((emili::vrp::VrpSolution*) base)->setSolution(solution);
    //((emili::vrp::VrpSolution*) base)->setUsedCapacity(used_capacity);
    ((emili::vrp::VrpSolution*) base )->removeEmptyVehicles();

  }
  ((emili::vrp::VrpSolution*) base)->setSolutionValue(fitness);
  ((emili::vrp::VrpSolution*) base)->setUsedCapacity(used_capacity);

}

/*-------------------------------------------------------------------------------------------------------*
 *--------------------------------------------------INSERTION--------------------------------------------*
 *-------------------------------------------------------------------------------------------------------*/

emili::Solution* emili::vrp::Random_Insertion::insertionArcs(emili::Solution* base){

  std::vector < std::vector<int> > solution = ((emili::vrp::VrpSolution*) base)->getSolution();
  std::vector<int> used_capacity =  ((emili::vrp::VrpSolution*) base)->getUsedCapacity();
  double fitness = ((emili::vrp::VrpSolution*) base)->getSolutionValue();

  int max_capacity = pis.getVehicleCapacity();

  std::vector<std::vector<double>> dis_matrix = pis.getDistanceMatrix();
  std::vector<double> dis_depot = pis.getDistanceWarehouses();
  std::vector<int> demands = pis.getDemands();


  std::vector <int> arc_insert = removalHeuristic->clients;

  int random_vehicle;
  int random_client;
  int insert_client = 0;
  int counter = 0;
  std::vector<int> empty_vector;

  while(arc_insert.size() > 0){
    random_vehicle = emili::generateRandomNumber() % solution.size();
    random_client = emili::generateRandomNumber() % arc_insert.size();
    counter++;
    if( (used_capacity[random_vehicle] + demands[solution[random_vehicle][random_client]] <= max_capacity) || counter >= 10 ){
      if(counter >= 10){
        solution.push_back(empty_vector);
        random_vehicle = solution.size() - 1;
      }
      insert_client = emili::generateRandomNumber() % solution[random_vehicle].size();
      if(insert_client == 0){
        fitness = fitness - dis_depot[solution[random_vehicle][0]] - dis_matrix[solution[random_vehicle][0]][solution[random_vehicle][1]]  + dis_depot[arc_insert[random_client]]; 
      }
      if(insert_client == solution[random_vehicle].size() - 1){
        fitness = fitness - dis_depot[solution[random_vehicle][solution[random_vehicle].size() - 1]] - dis_matrix[solution[random_vehicle].size() - 2][solution[random_vehicle].size() - 1]  + dis_depot[arc_insert[random_client]];
      }

      if(insert_client != 0){
        fitness = fitness + dis_matrix[ dis_depot[arc_insert[random_client]] ][ solution[random_vehicle][insert_client - 1]] - dis_matrix[solution[random_vehicle][insert_client]][solution[random_vehicle][insert_client - 1]];
      }

      if(insert_client != solution[random_vehicle].size() - 1){
        fitness = fitness + dis_matrix[ dis_depot[arc_insert[random_client]] ][ solution[random_vehicle][insert_client + 1]] - dis_matrix[solution[random_vehicle][insert_client]][solution[random_vehicle][insert_client + 1]];
      }

      solution[random_vehicle].insert(solution[random_vehicle].begin() + insert_client,1, arc_insert[random_client]);
      used_capacity[random_vehicle] += demands[arc_insert[insert_client]]; 
      arc_insert.erase(arc_insert.begin() + random_client);

    }
  }

  ((emili::vrp::VrpSolution*) base)->setSolution(solution);
  ((emili::vrp::VrpSolution*) base)->setSolutionValue(fitness);
  ((emili::vrp::VrpSolution*) base)->setUsedCapacity(used_capacity);

  return base;
}


/*----------------------------------------------------------------------------------------------------------------*
 *--------------------------------------------------PERTURBATION--------------------------------------------------*
 *----------------------------------------------------------------------------------------------------------------*/


emili::Solution* emili::vrp::RestartPerturbation::perturb(Solution *solution)
{
    emili::Solution* best;
    emili::Solution* current;
    best = initial->generateSolution();

    for(int i=0;i<num_of_solutions;i++)
    {
        current = initial->generateSolution();
        if( *current < *best)
        {
            delete best;
            best = current;
        }
        else
        {
            delete current;
        }

    }

    if(locser)
    {
        current = ls->search(best);
        if(current!=best)
            delete best;

        return current;
    }

    return best;
    //return solution;
}

emili::vrp::RestartPerturbation::~RestartPerturbation()
{
    if(ls!=nullptr)
        delete ls;
    delete initial;
}




emili::Solution* emili::vrp::TabuFitnessPerturbation::perturb(Solution *solution){
	/*std::cout <<"se cae"<<std::endl;
	emili::Solution *aux;
	aux = ((emili::vrp::VrpSolution*)solution)->clone();
	std::cout <<"se cae2"<<std::endl;*/
	/*double new_fitness = ((emili::vrp::VrpSolution*) solution)->getSolutionValue();
	bool is_tabu = false;
	emili::Solution *aux = ((emili::vrp::VrpSolution*)solution)->clone();
	if(tabu_list.size() == 0){
		tabu_list.push_back(new_fitness);
		last = ((emili::vrp::VrpSolution*)solution)->clone();
		return aux;
	}
	else{
		for(int i = 0; i < tabu_list.size(); i++){
			if(tabu_list[i] == new_fitness){
				is_tabu = true;
			}
		}

		if(is_tabu == false){
			last = ((emili::vrp::VrpSolution*)solution)->clone();
			tabu_list.push_back(new_fitness);

			while(tabu_list.size() > tenure_size){
				tabu_list.erase(tabu_list.begin());
			}
			return aux;
		}
		else{
			//solution = ((emili::vrp::VrpSolution*) last)->clone();
			solution = ((emili::vrp::VrpSolution*)last)->clone();
		}
	}
	//emili::Solution* new_solution = ((emili::vrp::VrpSolution*) solution)->clone();
	return aux;*/
  emili::Solution *aux = ((emili::vrp::VrpSolution*)solution)->clone();
  return aux;

}




emili::vrp::TabuFitnessPerturbation::~TabuFitnessPerturbation(){
    /*if(last !=nullptr){
        delete last;
    }*/
}




/*----------------------------------------------------------------------------------------------------------------*
 *-------------------------------------------------- ACCEPTANCE --------------------------------------------------*
 *----------------------------------------------------------------------------------------------------------------*/

emili::Solution* emili::vrp::TestAcceptance::accept(Solution *candidate1, Solution *candidate2)
{
    int chance = emili::generateRandomNumber()%100;
    emili::Solution* c = candidate1;
    emili::Solution* c2 = candidate2;
    if(candidate1->operator >(*candidate2)){
        c = candidate2;
        c2 = candidate1;
    }

    if(chance <= percentage)
    {
        return c;
    }
    else
    {
        return c2;
    }

}

emili::Solution* emili::vrp::SOAacceptance::accept(Solution *intensification_solution, Solution *diversification_solution)
{
    float intens = intensification_solution->getSolutionValue();
    float divers = diversification_solution->getSolutionValue();
    if(diversification_solution->operator >(*intensification_solution))
    {
        float prob = std::exp(100.0f*((intens-divers)/intens)/temperature);
        if(prob < 1.0 && emili::generateRealRandomNumber()>prob)
        {
            return intensification_solution;
        }
    }
    return diversification_solution;
}


emili::Solution* emili::vrp::SkewedAcceptanceDistance::accept(Solution *candidate1, Solution *candidate2)
{
    float distance_result = 0;
    emili::Solution* c = candidate1;
    emili::Solution* c2 = candidate2;

    std::vector<std::vector<int>>solution1 = ((emili::vrp::VrpSolution*) c)->getSolution();
    std::vector<std::vector<int>>solution2 = ((emili::vrp::VrpSolution*) c2)->getSolution();


    if(distance == 0){
      int u = 0;
      int size_sol = std::min(solution1.size(), solution2.size());
      int equal_vehicle = 1;

      for(int i = 0; i < size_sol; i++){

        if(solution1[i].size() == solution2[i].size()){

          for(int j = 0; j < solution1[i].size(); j++){
            if(solution1[i][j] != solution2[i][j]){
              equal_vehicle = 0;
              break;
            }

          }
        }
        u += equal_vehicle;
        equal_vehicle = 1;
      }

    }

    if( c2->getSolutionValue()  - c->getSolutionValue() < (alpha*(distance_result/problem.problemSize())) ){
      return c2;
    }
  return c;

}




/*----------------------------------------------------------------------------------------------------------------*
 *------------------------------------------------- TERMINATION --------------------------------------------------*
 *----------------------------------------------------------------------------------------------------------------*/

bool emili::vrp::TerminationIterations::terminate(Solution* currentSolution, Solution* newSolution)
{
    if(iterations < maxIterations)
    {
        if(newSolution != nullptr && currentSolution->operator <=(*newSolution))
        {
            iterations++;
        }

            return false;
    }
    return true;
}

void emili::vrp::TerminationIterations::reset()
{
    iterations=0;
}

bool emili::vrp::SOAtermination::terminate(Solution *currentSolution, Solution *newSolution)
{
    if(currentStep < numberOfSteps){
        currentStep++;
        return false;
    }
    else
    {
        return true;
    }
}

void emili::vrp::SOAtermination::reset()
{
    currentStep=0;
}



/*----------------------------------------------------------------------------------------------------------------*
 *---------------------------------------------------TABU LIST----------------------------------------------------*
 *----------------------------------------------------------------------------------------------------------------*/


/*-----------------------------
/-----TABU ALL SOLUTION--------
------------------------------*/

bool emili::vrp::TabuAllSolution::tabu_check(emili::Solution* solution){
	std::vector < std::vector<int> > solutionCurrent = ((emili::vrp::VrpSolution*) solution)->getSolution();
	int flagEqual = 1;
	int flag = 0;
	std::vector < std::vector<int> > aux;


	if(tabuVector.size() != 0){
                for(int i = 0; i < tabuVector.size(); i++){
			std::cout <<tabuVector[i]<< std::endl;
                        aux = ((emili::vrp::VrpSolution*) tabuVector[i])->getSolution();
                        if(aux.size() == solutionCurrent.size()){
				 for(int l = 0; l < aux.size(); l++){
                                 	if(aux[l].size() != solutionCurrent[l].size()){
                                        	flagEqual = 0;
                                 	}
                                 }
				/*for(int q = 0; q < aux.size(); q++){
                                	for(int l = 0; l < aux[q].size(); l++){
                                        	if(aux[l].size() != solutionCurrent[l].size()){
                                                	flagEqual = 0;
                                        	}
                                	}
				}*/
                        }
			else{
				flagEqual = 0;
			}
                        if(flagEqual == 1){
                                for(int j = 0; j < aux.size(); j++){
                                        for(int k = 0;k < aux[j].size(); k++){
						if(flag == 1){
							break;
						}
                                                if( aux[j][k] != solutionCurrent[j][k]  ){
                                                        flag = 1; // son distintos
                                                }
                                        }
                                }
                                if(flag == 0){
                                        return false;
                                }
                        }
                        flag = 0;
                        flagEqual = 1;
                }

	}
	return true;

}

void emili::vrp::TabuAllSolution::forbid(emili::Solution* solution){
	//std::vector < std::vector<int> > solutionCurrent = ((emili::vrp::VrpSolution*) solution)->getSolution();
	double valueCurrent = ((emili::vrp::VrpSolution*) solution)->getSolutionValue();
	if(tabuVector.size() == 0){
		tabuVector.push_back(solution->clone());
	}
	else{
		double Tabufirst = ((emili::vrp::VrpSolution*) tabuVector[0])->getSolutionValue();
		if(valueCurrent < Tabufirst){
			tabuVector.insert(tabuVector.begin(), solution->clone());
			if(tabuVector.size() > tabutenure){
				tabuVector.pop_back();
			}
		}
	}
}

void emili::vrp::TabuAllSolution::reset(){
	tabuVector.clear();
}

/*-------------------
/----TABU ONE CAR ---
--------------------*/

bool emili::vrp::TabuOneCar::tabu_check(emili::Solution* solution){
	//std::cout <<"holi" <<std::endl;
	std::vector < std::vector<int> > solutionCurrent = ((emili::vrp::VrpSolution*) solution)->getSolution();
	int flag = 0;
	for(int i = 0; i < solutionCurrent.size() ;i++){
		for(int j = 0; j < tabuVector.size() ; j++){
			if(i == tabuVector[j].first){
				if(solutionCurrent[i].size() == tabuVector[j].second.size()){
					for(int k = 0; k < tabuVector[j].second.size(); k++){
						if(solutionCurrent[i][k] != tabuVector[j].second[k]){
							flag = 1;
						}
					}
				}
			}
		}
	}
	if(flag == 1){
		return false;
	}
	//std::cout <<"holi" <<std::endl;
	return true;
}

void emili::vrp::TabuOneCar::forbid(emili::Solution* solution){
	std::vector < std::vector<int> > solutionCurrent = ((emili::vrp::VrpSolution*) solution)->getSolution();
	std::pair<int, std::vector<int>> tabuCar;
	int car = rand() % solutionCurrent.size();
	//std::cout <<"holi en fobid" <<std::endl;
	tabuCar.first  = car;
	tabuCar.second = solutionCurrent[car];
        double valueCurrent = ((emili::vrp::VrpSolution*) solution)->getSolutionValue();
//	std::cout <<"holi" <<std::endl;
        if(tabuVector.size() == 0){
                tabuVector.push_back(tabuCar);
		currentBest = valueCurrent;
        }
        else{
                if(valueCurrent < currentBest){
			currentBest = valueCurrent;
                        tabuVector.insert(tabuVector.begin(), tabuCar);
                        if(tabuVector.size() > tabutenure){
                                tabuVector.pop_back();
                        }
                }
        }
	//std::cout <<"chao en forbid" <<std::endl;
}

void emili::vrp::TabuOneCar::reset(){
        tabuVector.clear();
}

/*-----------------------------
/-----TABU ONE POSITION--------
------------------------------*/


bool emili::vrp::TabuOnePosition::tabu_check(emili::Solution* solution){
	std::vector < std::vector<int> > solutionCurrent = ((emili::vrp::VrpSolution*) solution)->getSolution();
	int flag = 1;
	for(int i = 0; i < tabuVector.size(); i++){
		if( (tabuVector[i].second.first ==_next_move.second.first)  && (tabuVector[i].second.second = _next_move.second.second) ){
			return false;
		}
	}
        return true;
}

void emili::vrp::TabuOnePosition::forbid(emili::Solution* solution){
	double valueCurrent = ((emili::vrp::VrpSolution*) solution)->getSolutionValue();
	if(tabuVector.size() == 0){
                tabuVector.push_back(_last_move);
        }
	else{
		if(valueCurrent < currentBest){
			 currentBest =  valueCurrent;
			 tabuVector.insert(tabuVector.begin(), _last_move);
			 if(tabuVector.size() > tabutenure){
			 	tabuVector.pop_back();
			 }
		}
        }
}

void emili::vrp::TabuOnePosition::setNeighborhood(emili::vrp::VrpNeighborhood* neighborhood){
	std::pair< std::pair<int, int>, std::pair<int,int> > _next_move =  (*neighborhood).next_move;
	std::pair< std::pair<int, int>, std::pair<int,int> > _last_move =  (*neighborhood).last_move;
	//std::vector < std::vector<int> > solutionCurrent = ((emili::vrp::VrpSolution*) solution)->getSolution();
	/*try
	{
		std::pair< std::pair<int, int>, std::pair<int,int> > _next_move =  ((emili::Neighborhood::VrpNeighborhood::InterRoute_Neighborhood*) neighborhood)->next_move;
        	std::pair< std::pair<int, int>, std::pair<int,int> > _last_move =  ((emili::Neighborhood::VrpNeighborhood::InterRoute_Neighborhood*) neighborhood)->last_move;
	}
	catch (const std::exception& e)
	{
		std::pair< std::pair<int, int>, std::pair<int,int> > _next_move =  ((emili::Neighborhood::VrpNeighborhood::IntraRoute_Neighborhood*) neighborhood)->next_move;
        	std::pair< std::pair<int, int>, std::pair<int,int> > _last_move =  ((emili::Neighborhood::VrpNeighborhood::IntraRoute_Neighborhood*) neighborhood)->last_move;
	}
	*/
}


void emili::vrp::TabuOnePosition::reset(){
        tabuVector.clear();
}

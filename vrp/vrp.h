
#ifndef VEHICULEROUTINGPROBLEM_H
#define VEHICULEROUTINGPROBLEM_H
#include "../emilibase.h"
#include "vrp_instance.h"
#include <vector>
/**
 *
 *  Vehicle Routing Problem components for EMILI
 *
*/
namespace emili{
namespace vrp{



/*----------------------------------------------------------------------------------------------------------------*
 *-------------------------------------PROBLEM & VRPPROBLEM TYPE CLASS--------------------------------------------*
 *----------------------------------------------------------------------------------------------------------------*/


class VrpProblem: public emili::Problem{
protected:
    VrpInstance instance;
public:
    // Constructor that uses a VrpInstance implementation
    VrpProblem(VrpInstance& problemInstance):instance(problemInstance) { }
    virtual ~VrpProblem(){};
    //Constructor that loads the instance from file path
    //VrpProblem(char* instance_path):instance(){ if (! instance.readDataFromFile(instance_path) ){exit(-1);}} //ramiro

    virtual double calcObjectiveFunctionValue(Solution& solution);
    virtual double evaluateSolution(Solution & solution);
    
    virtual int problemSize() {return getNcustomers();} //clients number

    /** Utils methods **/
    virtual double computeObjectiveFunction(std::vector < std::vector<int> > &solution)=0;

    /**  This method returns the number of customers*/
    int getNcustomers();
    /**  This method returns the capacity of the vehicles*/
    int getVehicleCapacity();
    /**  This method returns the demand of each client*/
    std::vector<int> getDemands();
    /** This method returns the matrix of distance between customers*/
    std::vector<std::vector<double>> getDistanceMatrix();
    /** This method returns the distance between customers and depot*/
    std::vector<double> getDistanceWarehouses();
};

/**  CLASSIC CVRP*/
class CVRP: public VrpProblem
{
public:
    CVRP(VrpInstance& problemInstance):VrpProblem(problemInstance) { }
    ~CVRP(){};

    virtual double computeObjectiveFunction(std::vector < std::vector<int> > & solution);
};



/*----------------------------------------------------------------------------------------------------------------------*
 *-------------------------------------SOLUTION & INITIALSOLUTION TYPE CLASS--------------------------------------------*
 *----------------------------------------------------------------------------------------------------------------------*/
struct vrpData {
  std::vector < std::vector<int>> routes;
  std::vector < int > used_capacity;
  vrpData (): routes(), used_capacity() {};
  vrpData (std::vector < std::vector<int>> r, std::vector<int> u): 
    routes(r), used_capacity(u){};
  ~vrpData() {} ;
};

class VrpSolution: public emili::Solution
{
protected:
    emili::vrp::vrpData data;
    std::vector< std::vector< int > > representation; //valores solucion vrp
    std::vector< std::vector< int > > last_representation; //backup of last representation
    double last_solution_value; //funcion objetivo
    
    std::vector<int> used_capacity;
    std::vector<int> last_used_capacity;
    
    bool last_backup;

    virtual const void* getRawData()const;
    virtual void setRawData(const void* data);
    
public:
  
    VrpSolution(double p_value):emili::Solution(p_value), last_backup(false)
    {}
  
    VrpSolution(double p_value, std::vector< std::vector< int > > & solution, std::vector <int> u_capacity):
      emili::Solution(p_value), representation(solution), used_capacity(u_capacity), last_backup(false), data(solution, u_capacity)
    {
    };
  
    virtual std::vector< std::vector< int > > & getSolution(); //retorna la representacion de la solucion
    
    virtual void setSolution(std::vector< std::vector< int > > & sol); //set the solution representation
    
    virtual double getSolutionValue(); //retorna valor de la solucion
    
    virtual void setSolutionValue(double solutionValue); //setea la solucion

    virtual std::string getSolutionRepresentation(); //imprime la representacion de la solucion
    
    virtual void printObjetivefunction(); //imprime la funcion objetivo
    
    virtual void printSolutionRepresentation();
      
    virtual Solution* clone();
    
    virtual bool operator==(Solution& a);
    
    virtual bool isFeasible(); 
    
    int getNVehicles () ;
    
    void removeEmptyVehicles ();
    
    virtual std::vector<int> getUsedCapacity();
    
    virtual void setUsedCapacity(std::vector<int> capacity);
    
    void reverse();

};

//The initial solution generator
class VrpInitialSolution: public emili::InitialSolution
{
protected:
    VrpProblem& pis;  //PROBLEM INSTANCE
    virtual Solution* generate() = 0 ;
public:
    VrpInitialSolution(VrpProblem& problem_instance):emili::InitialSolution(problem_instance),pis(problem_instance){};
    //virtual ~VrpInitialSolution() {};

    virtual Solution* generateSolution();
    virtual Solution* generateEmptySolution();

    //double calculateInitialSolutionValue(std::vector < std::vector<int> > &solution);
};

class VrpRandomInitialSolution: public emili::vrp::VrpInitialSolution
{
protected:
    virtual Solution* generate();
public:
    VrpRandomInitialSolution(VrpProblem& problem_instance):emili::vrp::VrpInitialSolution(problem_instance) {};
    //virtual ~VrpRandomInitialSolution() {};

    //double calculateInitialSolutionValue(std::vector < std::vector<int> > &solution);
};

class VrpDetInitialSolution: public emili::vrp::VrpInitialSolution
{
protected:
    virtual Solution* generate();
public:
    VrpDetInitialSolution(VrpProblem& problem_instance):emili::vrp::VrpInitialSolution(problem_instance){};
   // virtual ~VrpDetInitialSolution() {};

    //double calculateInitialSolutionValue(std::vector < std::vector<int> > &solution);
};



class VrpNNInitialSolution: public emili::vrp::VrpInitialSolution
{
protected:
    virtual Solution* generate();
public:
    VrpNNInitialSolution(VrpProblem& problem_instance):emili::vrp::VrpInitialSolution(problem_instance){};
   // virtual ~VrpDetInitialSolution() {};

    //double calculateInitialSolutionValue(std::vector < std::vector<int> > &solution);
};

class VrpFNInitialSolution: public emili::vrp::VrpInitialSolution
{
protected:
    virtual Solution* generate();
public:
    VrpFNInitialSolution(VrpProblem& problem_instance):emili::vrp::VrpInitialSolution(problem_instance){};
   // virtual ~VrpDetInitialSolution() {};

    //double calculateInitialSolutionValue(std::vector < std::vector<int> > &solution);
};

class VrpIMInitialSolution: public emili::vrp::VrpInitialSolution
{
protected:
    virtual Solution* generate();
public:
    VrpIMInitialSolution(VrpProblem& problem_instance):emili::vrp::VrpInitialSolution(problem_instance){};
   // virtual ~VrpDetInitialSolution() {};

    //double calculateInitialSolutionValue(std::vector < std::vector<int> > &solution);
};

class VrpSavingsInitialSolution: public emili::vrp::VrpInitialSolution
{
protected:
    virtual Solution* generate();
public:
    VrpSavingsInitialSolution(VrpProblem& problem_instance):emili::vrp::VrpInitialSolution(problem_instance){};
   // virtual ~VrpDetInitialSolution() {};

    //double calculateInitialSolutionValue(std::vector < std::vector<int> > &solution);
};



class VrpSavingsParam2InitialSolution: public emili::vrp::VrpInitialSolution
{
protected:
    double v_;
    double lambda_;
    virtual Solution* generate();
public:
    VrpSavingsParam2InitialSolution(VrpProblem& problem_instance, double v, double lambda):emili::vrp::VrpInitialSolution(problem_instance),v_(v),lambda_(lambda){};
   // virtual ~VrpDetInitialSolution() {};

    //double calculateInitialSolutionValue(std::vector < std::vector<int> > &solution);
};
class VrpSavingsParam1InitialSolution: public emili::vrp::VrpInitialSolution
{
protected:
	virtual Solution* generate();
public:
	VrpSavingsParam1InitialSolution(VrpProblem& problem_instance):emili::vrp::VrpInitialSolution(problem_instance){};
};


class ChristofidesInitialSolution: public emili::vrp::VrpInitialSolution
{
protected:
	virtual Solution* generate();

	virtual int getMinIndex(std::vector<int> key, std::vector<bool> included);
	virtual std::vector<int> findOdds(std::vector<int> adjlist);
	virtual int findBestPath(int t,std::vector<int> adjlist);
	virtual void euler_tour(int start,std::vector<int> &path,std::vector<int> adjlist);
	virtual void make_hamiltonian(std::vector<int> &path, int &pathCost);

public:
	ChristofidesInitialSolution(VrpProblem& problem_instance):emili::vrp::VrpInitialSolution(problem_instance){};


};

class LocalSearchInitial: public emili::vrp::VrpInitialSolution
{
protected:
	virtual Solution* generate();

  emili::LocalSearch * lls;
public:
	LocalSearchInitial(VrpProblem& problem_instance,LocalSearch& lls):emili::vrp::VrpInitialSolution(problem_instance){
    this ->lls = &lls;
  };


};

/*--------------------------------------------------------------------------------------------------------*
 *-------------------------------------NEIGHBORHOOD TYPE CLASS--------------------------------------------*
 *--------------------------------------------------------------------------------------------------------*/

/* INTER-NEIGHBORHOOD */
class VrpNeighborhood: public emili::Neighborhood
{
protected:
    //std::pair< std::pair<int, int>, std::pair<int,int> > next_move;
    //std::pair< std::pair<int, int>, std::pair<int,int> > last_move;
    VrpProblem& pis;
    int n_customers;

public:
    std::pair< std::pair<int, int>, std::pair<int,int> > next_move;
    std::pair< std::pair<int, int>, std::pair<int,int> > last_move;
    VrpNeighborhood(VrpProblem& problem):pis(problem), n_customers(problem.getNcustomers()) {};
    virtual VrpProblem& getProblem() { return pis; }
};

class InterRoute_Neighborhood: public emili::vrp::VrpNeighborhood
{
protected:
  // Format <origin_vehicle, origin_index> <destination_vehicle, destination_index>
  std::pair< std::pair<int, int>, std::pair<int,int> > next_move;
  std::pair< std::pair<int, int>, std::pair<int,int> > last_move;
  // Format <pre_origin_custumer, post_origin_customer> <pre_dest_customer, post_dest_customer>
  std::pair< std::pair<int, int>, std::pair<int,int> > pre_post_customers;
  int n_size;
  int n_iterations; //counter of neighbors evaluated from begin() or reset()
  int c_iterations; //counter of neighbors evaluated to change only one customer (or set of customers)
  int last_n_iterations; // variable to revert
  int last_c_iterations; // variable to revert
  int last_n_size; // variable to revert
  bool isFirstTime ; //indicates first neighbor evaluated after a solution change
  
  virtual Solution* computeStep(Solution* step);
  virtual void reverseLastMove(Solution* step);
  virtual void setNextNeighborPos(emili::Solution* base)=0;
  virtual void setAuxiliaryVariables(std::vector < std::vector<int> > & solution) = 0;
  virtual void calculateNeighborhoodSize(emili::Solution* base) = 0;
  virtual void makeMove(emili::Solution* base) = 0;
  virtual bool isNextFeasible(emili::Solution* base) = 0;
  
public:
  InterRoute_Neighborhood(VrpProblem& problem): VrpNeighborhood(problem), n_iterations(0), c_iterations(0) {};
  virtual NeighborhoodIterator begin(emili::Solution* base);
  virtual Solution* random(Solution* currentSolution) = 0;
  /*virtual Solution* random(Solution *currentSolution, int size);*/
  virtual void reset();
  virtual std::pair<std::pair<int,int>, std::pair<int,int>> lastMove() { return last_move;};
  virtual int size() { return n_size;};  
};

class Shift_1_0_Neighborhood: public emili::vrp::InterRoute_Neighborhood
{
protected:
  void setNextNeighborPos(emili::Solution* base);
  void setAuxiliaryVariables(std::vector < std::vector<int> > & solution);
  bool isNextFeasible(emili::Solution* base);
  void makeMove(emili::Solution* base);
  double calculateDeltaSolutionValue(emili::Solution* base);
  void calculateNeighborhoodSize(emili::Solution* base);
public:
  Shift_1_0_Neighborhood(VrpProblem& problem): InterRoute_Neighborhood(problem){};
  Solution* random(Solution* currentSolution);
};

class Shift_1_0_Neighborhood_With_Reparation: public emili::vrp::InterRoute_Neighborhood
{
protected:
  void setNextNeighborPos(emili::Solution* base);
  void setAuxiliaryVariables(std::vector < std::vector<int> > & solution);
  bool isNextFeasible(emili::Solution* base);
  void makeMove(emili::Solution* base);
  double calculateDeltaSolutionValue(emili::Solution* base);
  void calculateNeighborhoodSize(emili::Solution* base);
public:
  Shift_1_0_Neighborhood_With_Reparation(VrpProblem& problem): InterRoute_Neighborhood(problem){};
  Solution* random(Solution* currentSolution);
  void repair(emili::Solution* base);
};


class Shift_2_0_Neighborhood: public emili::vrp::InterRoute_Neighborhood
{
protected:
  void setNextNeighborPos(emili::Solution* base);
  void setAuxiliaryVariables(std::vector < std::vector<int> > & solution);
  bool isNextFeasible(emili::Solution* base);
  void makeMove(emili::Solution* base);
  double calculateDeltaSolutionValue(emili::Solution* base);
  void calculateNeighborhoodSize(emili::Solution* base);
public:
  Shift_2_0_Neighborhood(VrpProblem& problem): InterRoute_Neighborhood(problem){};
  Solution* random(Solution* currentSolution);
};

class Shift_2_0_Neighborhood_With_Reparation: public emili::vrp::InterRoute_Neighborhood
{
protected:
  void setNextNeighborPos(emili::Solution* base);
  void setAuxiliaryVariables(std::vector < std::vector<int> > & solution);
  bool isNextFeasible(emili::Solution* base);
  void makeMove(emili::Solution* base);
  double calculateDeltaSolutionValue(emili::Solution* base);
  void calculateNeighborhoodSize(emili::Solution* base);
public:
  Shift_2_0_Neighborhood_With_Reparation(VrpProblem& problem): InterRoute_Neighborhood(problem){};
  Solution* random(Solution* currentSolution);
  void repair(emili::Solution* base);
};


class Swap_1_1_Neighborhood: public emili::vrp::InterRoute_Neighborhood
{
protected:
  void setNextNeighborPos(emili::Solution* base);
  void setAuxiliaryVariables(std::vector < std::vector<int> > & solution);
  bool isNextFeasible(emili::Solution* base);
  void makeMove(emili::Solution* base);
  double calculateDeltaSolutionValue(emili::Solution* base);
  void calculateNeighborhoodSize(emili::Solution* base);
public:
  Swap_1_1_Neighborhood(VrpProblem& problem): InterRoute_Neighborhood(problem){};
  Solution* random(Solution* currentSolution);
  /*virtual Solution* random(Solution *currentSolution, int size);*/
};


class Swap_1_1_Neighborhood_With_Reparation: public emili::vrp::InterRoute_Neighborhood
{
protected:
  void setNextNeighborPos(emili::Solution* base);
  void setAuxiliaryVariables(std::vector < std::vector<int> > & solution);
  bool isNextFeasible(emili::Solution* base);
  void makeMove(emili::Solution* base);
  double calculateDeltaSolutionValue(emili::Solution* base);
  void calculateNeighborhoodSize(emili::Solution* base);
public:
  Swap_1_1_Neighborhood_With_Reparation(VrpProblem& problem): InterRoute_Neighborhood(problem){};
  Solution* random(Solution* currentSolution);
  void repair(emili::Solution* base);
  /*virtual Solution* random(Solution *currentSolution, int size);*/
};

class Swap_2_1_Neighborhood: public emili::vrp::InterRoute_Neighborhood
{
protected:
  void setNextNeighborPos(emili::Solution* base);
  void setAuxiliaryVariables(std::vector < std::vector<int> > & solution);
  bool isNextFeasible(emili::Solution* base);
  void makeMove(emili::Solution* base);
  double calculateDeltaSolutionValue(emili::Solution* base);
  void calculateNeighborhoodSize(emili::Solution* base);
public:
  Swap_2_1_Neighborhood(VrpProblem& problem): InterRoute_Neighborhood(problem){};
  Solution* random(Solution* currentSolution);
};

class Swap_2_1_Neighborhood_With_Reparation: public emili::vrp::InterRoute_Neighborhood
{
protected:
  void setNextNeighborPos(emili::Solution* base);
  void setAuxiliaryVariables(std::vector < std::vector<int> > & solution);
  bool isNextFeasible(emili::Solution* base);
  void makeMove(emili::Solution* base);
  double calculateDeltaSolutionValue(emili::Solution* base);
  void calculateNeighborhoodSize(emili::Solution* base);
public:
  Swap_2_1_Neighborhood_With_Reparation(VrpProblem& problem): InterRoute_Neighborhood(problem){};
  Solution* random(Solution* currentSolution);
  void repair(emili::Solution* base);
};

class Swap_2_2_Neighborhood: public emili::vrp::InterRoute_Neighborhood
{
protected:
  void setNextNeighborPos(emili::Solution* base);
  void setAuxiliaryVariables(std::vector < std::vector<int> > & solution);
  bool isNextFeasible(emili::Solution* base);
  void makeMove(emili::Solution* base);
  double calculateDeltaSolutionValue(emili::Solution* base);
  void calculateNeighborhoodSize(emili::Solution* base);
public:
  Swap_2_2_Neighborhood(VrpProblem& problem): InterRoute_Neighborhood(problem){};
  Solution* random(Solution* currentSolution);
};

class Swap_2_2_Neighborhood_With_Reparation: public emili::vrp::InterRoute_Neighborhood
{
protected:
  void setNextNeighborPos(emili::Solution* base);
  void setAuxiliaryVariables(std::vector < std::vector<int> > & solution);
  bool isNextFeasible(emili::Solution* base);
  void makeMove(emili::Solution* base);
  double calculateDeltaSolutionValue(emili::Solution* base);
  void calculateNeighborhoodSize(emili::Solution* base);
public:
  Swap_2_2_Neighborhood_With_Reparation(VrpProblem& problem): InterRoute_Neighborhood(problem){};
  Solution* random(Solution* currentSolution);
  void repair(emili::Solution* base);
};

class Cross_Exchange_Neighborhood: public emili::vrp::InterRoute_Neighborhood
{
protected:
  void setNextNeighborPos(emili::Solution* base);
  void setAuxiliaryVariables(std::vector < std::vector<int> > & solution);
  bool isNextFeasible(emili::Solution* base);
  void makeMove(emili::Solution* base);
  double calculateDeltaSolutionValue(emili::Solution* base);
  void calculateNeighborhoodSize(emili::Solution* base);
public:
  std::pair< std::pair<int, std::pair<int,int> > ,  std::pair<int, std::pair<int,int> > > next_move;
  std::pair< std::pair<int, std::pair<int,int> > ,  std::pair<int, std::pair<int,int> > > last_move;
  // Format <pre_origin_custumer, post_origin_customer> <pre_dest_customer, post_dest_customer>
  std::pair< std::pair<int, std::pair<int,int> > ,  std::pair<int, std::pair<int,int> > > pre_post_customers;
  Cross_Exchange_Neighborhood(VrpProblem& problem): InterRoute_Neighborhood(problem){};
  Solution* random(Solution* currentSolution);
};




/* INTRA-NEIGHBORHOOD */

class IntraRoute_Neighborhood: public emili::vrp::VrpNeighborhood
{
protected:
  // Format <origin_vehicle, origin_index> <destination_vehicle, destination_index>
  std::pair< int, std::pair<int,int> > next_move;
  std::pair< int, std::pair<int,int> > last_move;
  // Format <pre_origin_custumer, post_origin_customer> <pre_dest_customer, post_dest_customer>
  std::pair< std::pair<int, int>, std::pair<int,int> > pre_post_customers;
  int n_size;
  int n_iterations;
  int r_iterations;
  int last_n_iterations;
  int last_r_iterations;
  int last_n_size;
  bool isFirstTime ;
  
  virtual Solution* computeStep(Solution* step);
  virtual void setNextNeighborPos(emili::Solution* base)=0;  
  virtual void makeMove(emili::Solution* base)=0;
  virtual bool isNextFeasible(emili::Solution* base)=0;
  virtual void calculateNeighborhoodSize(emili::Solution* base)=0;
  virtual void reverseLastMove(Solution* step);
  
public:
  IntraRoute_Neighborhood(VrpProblem& problem): VrpNeighborhood(problem), n_iterations(0), r_iterations(0) {};
  
  virtual NeighborhoodIterator begin(emili::Solution* base);
  virtual void reset();
  virtual Solution* random(Solution* currentSolution) = 0;
  virtual std::pair< int, std::pair<int,int>> lastMove() { return last_move;};
  virtual int size() { return n_size;};
};

class Reinsertion_Neighborhood: public emili::vrp::IntraRoute_Neighborhood
{
protected:
  virtual void setNextNeighborPos(emili::Solution* base);
  virtual void setAuxiliaryVariables(std::vector < std::vector<int> > & solution) ;
  virtual void calculateNeighborhoodSize(emili::Solution* base);
  virtual void makeMove(emili::Solution* base);
  virtual bool isNextFeasible(emili::Solution* base);
  virtual double calculateDeltaSolutionValue(emili::Solution* base);
  
public:
  Reinsertion_Neighborhood(VrpProblem& problem): IntraRoute_Neighborhood(problem){};
  virtual Solution* random(Solution* currentSolution);
  //virtual void reset();
};

class Exchange_Neighborhood: public emili::vrp::IntraRoute_Neighborhood
{
protected:
  virtual void setNextNeighborPos(emili::Solution* base);
  virtual void setAuxiliaryVariables(std::vector < std::vector<int> > & solution) ;
  virtual void calculateNeighborhoodSize(emili::Solution* base);
  virtual void makeMove(emili::Solution* base);
  virtual bool isNextFeasible(emili::Solution* base);
  virtual double calculateDeltaSolutionValue(emili::Solution* base);
  
public:
  Exchange_Neighborhood(VrpProblem& problem): IntraRoute_Neighborhood(problem){};
  
  //virtual NeighborhoodIterator begin(emili::Solution* base);
  virtual Solution* random(Solution* currentSolution);
  //virtual void reset();

};

class TwoOPT_Neighborhood: public emili::vrp::IntraRoute_Neighborhood
{
protected:
  virtual void setNextNeighborPos(emili::Solution* base);
  virtual void setAuxiliaryVariables(std::vector < std::vector<int> > & solution) ;
  virtual void calculateNeighborhoodSize(emili::Solution* base);
  virtual void makeMove(emili::Solution* base);
  virtual bool isNextFeasible(emili::Solution* base);
  virtual double calculateDeltaSolutionValue(emili::Solution* base);
  
public:
  TwoOPT_Neighborhood(VrpProblem& problem): IntraRoute_Neighborhood(problem){};
  
  //virtual NeighborhoodIterator begin(emili::Solution* base);
  virtual Solution* random(Solution* currentSolution);
  //virtual void reset();
  
};


class Removal_Insertion_Neighborhood: public emili::vrp::IntraRoute_Neighborhood
{
protected:
  virtual void setNextNeighborPos(emili::Solution* base);
  virtual void setAuxiliaryVariables(std::vector < std::vector<int> > & solution) ;
  virtual void calculateNeighborhoodSize(emili::Solution* base);
  virtual void makeMove(emili::Solution* base);
  virtual bool isNextFeasible(emili::Solution* base);
  virtual double calculateDeltaSolutionValue(emili::Solution* base);
  
public:
  Removal_Insertion_Neighborhood(VrpProblem& problem): IntraRoute_Neighborhood(problem){};
  
  //virtual NeighborhoodIterator begin(emili::Solution* base);
  virtual Solution* random(Solution* currentSolution);
  //virtual void reset();
  
};

/*-------------------------------------------------------------------------------------------------------*
 *----------------------------------------------------REMOVAL--------------------------------------------*
 *-------------------------------------------------------------------------------------------------------*/


class Random_Removal: public emili::Removal
{
  protected:
    VrpProblem& pis;
    int n_customers;

  public:
    Random_Removal(VrpProblem& problem,int arcs):Removal(arcs), pis(problem),n_customers(problem.getNcustomers()) {}
    virtual void removalArcs(emili::Solution* base);
};

/*-------------------------------------------------------------------------------------------------------*
 *--------------------------------------------------INSERTION--------------------------------------------*
 *-------------------------------------------------------------------------------------------------------*/

class Random_Insertion: public emili::Insertion
{
  protected:
    VrpProblem& pis;
    int n_customers;

  public:
    Random_Insertion(VrpProblem& problem, emili::Removal* removalHeuristic): Insertion(removalHeuristic), pis(problem), n_customers(problem.getNcustomers()) {}    
    virtual emili::Solution* insertionArcs(emili::Solution* base);
};

/*-------------------------------------------------------------------------------------------------------*
 *--------------------------------------------------TABU LIST--------------------------------------------*
 *-------------------------------------------------------------------------------------------------------*/

class TabuAllSolution: public emili::TabuMemory
{
public:
    int tabutenure;
    std::vector<Solution *> tabuVector;

    TabuAllSolution(int tenureSize):tabutenure(tenureSize) { }
    TabuAllSolution():tabutenure(1) { }

    virtual bool tabu_check(emili::Solution* solution);  // revisa si la solucion. esta en la lista tabu
    virtual void forbid(emili::Solution* solution); // marca la entrada como tabu
    //virtual void registerMove(emili::Solution* base,emili::Solution* solution);// se llama para guardar la solucion en la lista tabu
    virtual void reset(); // resetea el objeto, entre 2 busquedas distintas
};

class TabuOneCar: public emili::TabuMemory
{
protected:
    int tabutenure;
    double currentBest;
    std::vector<std::pair<int, std::vector<int>>> tabuVector;
public:
    TabuOneCar(int tenureSize):tabutenure(tenureSize) { }
    TabuOneCar():tabutenure(1) { }

    virtual bool tabu_check(emili::Solution* solution);  // revisa si la solucion. esta en la lista tabu
    virtual void forbid(emili::Solution* solution); // marca la entrada como tabu
   // virtual void registerMove(emili::Solution* base,emili::Solution* solution);// se llama para guardar la solucion en la lista tabu
    virtual void reset(); // resetea el objeto, entre 2 busquedas distintas
};

class TabuOnePosition: public emili::TabuMemory
{
protected:
    int tabutenure;
    double currentBest;
    std::pair< std::pair<int, int>, std::pair<int,int> > _next_move;
    std::pair< std::pair<int, int>, std::pair<int,int> > _last_move;
    std::vector< std::pair< std::pair<int, int>, std::pair<int,int> >> tabuVector;
public:
    TabuOnePosition(int tenureSize):tabutenure(tenureSize) { }
    TabuOnePosition():tabutenure(1) { }

    virtual bool tabu_check(emili::Solution* solution);  // revisa si la solucion. esta en la lista tabu
    virtual void forbid(emili::Solution* solution); // marca la entrada como tabu
   // virtual void registerMove(emili::Solution* base,emili::Solution* solution);// se llama para guardar la solucion en la lista tabu
    virtual void setNeighborhood(emili::vrp::VrpNeighborhood* neighborhood);
    virtual void reset(); // resetea el objeto, entre 2 busquedas distintas
};


/*-------------------------------------------------------------------------------------------------------*
 *-------------------------------------PERTUBATION TYPE CLASS--------------------------------------------*
 *-------------------------------------------------------------------------------------------------------*/

class RestartPerturbation: public emili::Perturbation
{
protected:
    int num_of_solutions;
    bool locser;
    emili::InitialSolution* initial;
    emili::LocalSearch* ls;
public:
    RestartPerturbation(int np, emili::InitialSolution* init, emili::LocalSearch* ll):num_of_solutions(np),initial(init),ls(ll),locser(true) {}
    RestartPerturbation(int np, emili::InitialSolution* init):num_of_solutions(np),initial(init),ls(nullptr),locser(false) {}

    emili::Solution* perturb(Solution *solution);
    ~RestartPerturbation();
};


class TabuFitnessPerturbation: public emili::Perturbation
{
protected:
	//int tenure_size;
	std::vector<double> tabu_list;
	emili::Solution* last;
  emili::TabuMemory* tabulist;
public:
	TabuFitnessPerturbation(emili::TabuMemory* tabulist): tabulist(tabulist){}
	emili::Solution* perturb(Solution *solution);
	~TabuFitnessPerturbation();
};


/*-------------------------------------------------------------------------------------------------------*
 *----------------------------------------- ACCEPTANCE CLASS --------------------------------------------*
 *-------------------------------------------------------------------------------------------------------*/

class TestAcceptance: public emili::Acceptance
{
protected:
    VrpProblem& problem;
    int percentage;
public:
    TestAcceptance(VrpProblem& problem_instance):problem(problem_instance),percentage(70) { }
    TestAcceptance(VrpProblem& problem_instance,int perc):problem(problem_instance),percentage(perc) { }
    virtual Solution* accept(Solution* candidate1, Solution* candidate2);
};

class SOAacceptance: public emili::MetropolisAcceptance
{
public:
    SOAacceptance(float start_temp):emili::MetropolisAcceptance(start_temp) { }
    virtual Solution* accept(Solution* candidate1, Solution* candidate2);
};

class SkewedAcceptanceDistance: public emili::Acceptance
{
protected:
  VrpProblem& problem;
  float alpha;
  int distance;
  
public:
    SkewedAcceptanceDistance(VrpProblem& problem_instance,float _alpha, int _distance):problem(problem_instance),alpha(_alpha),distance(_distance) { }
    virtual Solution* accept(Solution* candidate1, Solution* candidate2);
};

/*-------------------------------------------------------------------------------------------------------*
 *----------------------------------------- TERMINATION CLASS -------------------------------------------*
 *-------------------------------------------------------------------------------------------------------*/
 
class TerminationIterations: public emili::Termination
{
protected:
    int iterations;
    int maxIterations;
public:
    TerminationIterations(int max_badIterations):maxIterations(max_badIterations),iterations(0) { }
    virtual bool terminate(Solution* currentSolution, Solution* newSolution);
    void reset();
};

class SOAtermination: public emili::Termination
{
protected:
    int numberOfSteps;
    int currentStep;
public:
    SOAtermination(int number_of_steps):numberOfSteps(number_of_steps),currentStep(0) { }
    virtual bool terminate(Solution* currentSolution, Solution* newSolution);
    void reset();
};



}
}
#endif // VEHICULEROUTINGPROBLEM_H

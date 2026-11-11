

#include "vrp_builder.h"
#include "vrp.h"
#include "vrp_instance.h"
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <cstring>
#include <iostream>
#include <sstream>

/*---------------------------------------------------------------------------------------------------------------------*
 *-------------------------------------DEFINICION DE SECCIONES DEL PROBLEMA--------------------------------------------*
 *---------------------------------------------------------------------------------------------------------------------*/

#define EMPTY_LS "vacio"
#define TEST_LS "test"


/* 
 *  Vehicule Routing Problem 
 */

#define PROBLEM_CAPACITATED_VRP "CVRP"

/* 
 *initial solution heuristics 
 */

#define INITIAL_RANDOM_VRP "random"
#define INITIAL_DET_VRP "determinist"
#define INITIAL_NN_VRP "nearest_n"
#define INITIAL_FN_VRP "farthest_n"
#define INITIAL_IM_VRP "insertion_m"
#define INITIAL_SA_VRP "savings_alg"
#define INITIAL_SA_VRP_PAR1 "savings_alg_par1"
#define INITIAL_SA_VRP_PAR2 "savings_alg_par2"
#define INITIAL_CHRISTOFIDES "christofides"
#define INITIAL_LOCAL_SEARCH "initial_ls"
/*
 *  vehicule routing problem neighborhoods
 */

/* Inter route structures */
#define NEIGHBORHOOD_INTER_ROUTE_SHIFT_1_0 "nshift10"
#define NEIGHBORHOOD_INTER_ROUTE_SHIFT_1_0_REPAIR "repair_nshift10"

#define NEIGHBORHOOD_INTER_ROUTE_SHIFT_2_0 "nshift20"
#define NEIGHBORHOOD_INTER_ROUTE_SHIFT_2_0_REPAIR "repair_nshift20"

#define NEIGHBORHOOD_INTER_ROUTE_SWAP_1_1 "nswap11"
#define NEIGHBORHOOD_INTER_ROUTE_SWAP_1_1_REPAIR "repair_nswap11"

#define NEIGHBORHOOD_INTER_ROUTE_SWAP_2_1 "nswap21"
#define NEIGHBORHOOD_INTER_ROUTE_SWAP_2_1_REPAIR "repair_nswap21"

#define NEIGHBORHOOD_INTER_ROUTE_SWAP_2_2 "nswap22"
#define NEIGHBORHOOD_INTER_ROUTE_SWAP_2_2_REPAIR "repair_nswap22"

#define NEIGHBORHOOD_INTER_ROUTE_CROSS_EXCHANGE "ncrossexchange"
#define NEIGHBORHOOD_INTER_ROUTE_CROSS "ncross"
/* Intra route structures */
#define NEIGHBORHOOD_INTRA_ROUTE_REINSERTION "nrein"
#define NEIGHBORHOOD_INTRA_ROUTE_EXCHANGE "nexchange"
#define NEIGHBORHOOD_INTRA_ROUTE_OR_OPT_2 "n2opt"
#define NEIGHBORHOOD_INTRA_ROUTE_OR_OPT_3 "nopt3"


/* 
 *removal heuristics 
 */

#define REMOVAL_RANDOM_HEURISTIC "removal_insertion"

/* 
 *insertion heuristics 
 */
#define INSERTION_RANDOM_HEURISTIC "random_insertion"

/* 
 *perturbations types 
 */

#define PERTURBATION_MULTI_SWAP_1_1 "p_ms_w_11"
#define PERTURBATION_MULTI_SHIFT_1_1 "p_ms_h_11"
#define PERTURBATION_RESTART "restart"
#define PERTURBATION_RESTART_LS "restartls"
#define PERTURBATION_TABU "p_tabu"

/* 
 *acceptance criteria 
 */
#define ACCEPTANCE_TEST "testacc"
#define ACCEPTANCE_SOA "soaacc"
#define ACCEPTANCE_SKEWED "skewedacc"

 /* 
 *termination criteria 
 */
#define TERMINATION_ITERA "iteration"
#define TERMINATION_SOA "soater"

/*
 *tabu list
 */
#define TABU_MEMORY_ALL_SOLUTION "Tabu_all_solution"
#define TABU_MEMORY_1_CAR "Tabu_1_car"
#define TABU_ONE_POSITION "Tabu_1_position"





/*--------------------------------------------------------------------------------------------------------------------*
 *------------------------------------ INSTANCE LOADING AND INITIALIZATION -------------------------------------------*
 *--------------------------------------------------------------------------------------------------------------------*/

char* problem_string;
emili::vrp::VrpProblem* loadProblem(char* t, VrpInstance i);


//------------------------------ execution information ------------------------------
std::string info_vrp()
{
    std::ostringstream oss;
    oss << "Usage:\n\n";
    oss << "PROBLEM               = "<<PROBLEM_CAPACITATED_VRP <<"\n";
    oss << "LOCAL_SEARCH          = SEARCH_TYPE INITIAL_SOLUTION TERMINATION NEIGHBORHOOD" <<"\n";
    oss << "ITERATED_LOCAL_SEARCH = ils LOCAL_SEARCH TERMINATION PERTURBATION ACCEPTANCE -it seconds" << "\n";
    oss << "TABU_SEARCH           = tabu < first | best > INITIAL_SOLUTION TERMINATION NEIGHBORHOOD TABU_MEMORY " << "\n";
    oss << "GVNS_SEARCH           = gvns INITIAL_SOLUTION PERTURBATION1 PERTURBATION2 -it seconds" << "\n";
    oss << "INITIAL_SOLUTION      = random | determinist | nearest_n" <<"\n";
    oss << "TERMINATION           = true | time float | locmin | soater | iteration int | maxstep int" << "\n";
    oss << "NEIGHBORHOOD          = transpose | exchange | insert | binsert | finsert | tinsert | nswap22\n";
    oss << "PERTURBATION           = p_tabu int| p_ms_h_11 | p_ms_w_11 |igper int | testper | rndmv NEIGHBORHOOD #moves(int) | perset NEIGHBORHOOD | noper (int) | nrzper (int) | tmiigper (int) (int) | igls (int) LOCAL_SEARCH | rsls (int) LOCAL_SEARCH" << "\n";
    oss << "ACCEPTANCE            = soaacc float | testacc #swaps(int) | metropolis start_temperature(float) | always (intensify | diversify) | improve | sa_metropolis start_temp end_temp ratio | pmetro start_temp end_temp ratio frequence(int) | saacc start_temp end_temp ratio frequence(int) alpha ]0,1] | tmiigacc start_temperature(float) | implat number_of_non_improving_steps_accepted plateau_threshold" << std::endl;
    oss << "TABU_MEMORY           = Tabu_all_solution size(int) | Tabu_1_car size(int) | Tabu_1_position size(int) "<< "\n";
    //oss << "TABU_MEMORY           = Tabu_all_solution size(int) "<< "\n";
    return oss.str();
}

/*------------------------------ creation of algorithms------------------------------
emili::LocalSearch* prs::vrp::VrpBuilder::buildAlgo()
{
    prs::incrementTabLevel();
    emili::LocalSearch* ls = nullptr;
    if(tm.checkToken(TEST_LS))
    {
        printTab("LOCAL SEARCH");
        emili::vrp::VrpProblem* instance =(emili::vrp::VrpProblem*) gp.getInstance();
        emili::InitialSolution* in = retrieveComponent(COMPONENT_INITIAL_SOLUTION_GENERATOR).get<emili::InitialSolution>();
        emili::Termination* te = retrieveComponent(COMPONENT_TERMINATION_CRITERION).get<emili::Termination>();
        emili::Neighborhood* ne = retrieveComponent(COMPONENT_NEIGHBORHOOD).get<emili::Neighborhood>();
        ls = new emili::vrp::SinLS(*instance,*in);
    }
    prs::decrementTabLevel();
    return ls;
}
*/

//------------------------------ read and load instance ------------------------------
emili::Problem* prs::vrp::VrpBuilder::openInstance()
{

    VrpInstance instance;
    problem_string = tm.nextToken(); //avanza al siguiente token que corresponde a la direccion de la instancia
    bool ok;

    ok = instance.readDataFromFile(tm.tokenAt(1)); //true si existe el token, falso si no

    if(ok) //si existe la instancia llama a loadproblem que carga el problema
    {
       emili::vrp::VrpProblem* problem = loadProblem(problem_string, instance);
       return problem;
    }

    std::cout << info_vrp() << std::endl;
    exit(-1);
}

/*/------------------------------problem creation------------------------------
emili::Problem* prs::PfspBuilder::buildProblem()
{
    emili::vrp::VrpProblem* instance =(emili::vrp::VrpProblem*) gp.getInstance();
    return loadProblem(tm.nextToken(),instance->getInstance());
}
*/

//------------------------------reading of the problem------------------------------
emili::vrp::VrpProblem* loadProblem(char* t, VrpInstance i)
{
  emili::vrp::VrpProblem* prob;
  if(strcmp(t,PROBLEM_CAPACITATED_VRP)==0)
  {
      //prs::printTab("Capacitated Vehicule Routing Problem");
      prob = new emili::vrp::CVRP(i);
  }
  else
  {
      std::cerr<< "'" << t << "' -> ERROR a problem was expected! " << std::endl;
      prs::info();
      exit(-1);
  }
  return prob;
}



/*----------------------------------------------------------------------------------------------------------------------*
 *---------------------------------------------- ALGORITHMIC COMPONENTS ------------------------------------------------*
 *----------------------------------------------------------------------------------------------------------------------*/


//------------------------------ initial solution ------------------------------
emili::InitialSolution* prs::vrp::VrpBuilder::buildInitialSolution()
{
    prs::incrementTabLevel();
    emili::InitialSolution* init = nullptr;
    emili::vrp::VrpProblem* instance =(emili::vrp::VrpProblem*) gp.getInstance();
    
    //la solucion inicial varia dependiendo de los parámetros ingresados por consola
    if(tm.checkToken(INITIAL_RANDOM_VRP))
    {
        printTab( "Random initial solution");
        init = new emili::vrp::VrpRandomInitialSolution(*instance);
    }
    else if (tm.checkToken(INITIAL_DET_VRP))
    {
        printTab( "Determinist initial solution");
        init = new emili::vrp::VrpDetInitialSolution(*instance);
    }
    else if (tm.checkToken(INITIAL_NN_VRP))
    {
        printTab( "nearest neighbor initial solution");
        init = new emili::vrp::VrpNNInitialSolution(*instance);
    }
    else if (tm.checkToken(INITIAL_FN_VRP))
    {
        printTab( "farthest neighbor initial solution");
        init = new emili::vrp::VrpFNInitialSolution(*instance);
    }
    else if (tm.checkToken(INITIAL_IM_VRP))
    {
        printTab( "Insertion method initial solution");
        init = new emili::vrp::VrpIMInitialSolution(*instance);
    }
    else if (tm.checkToken(INITIAL_SA_VRP))
    {
        printTab( "clarke and Wright savings initial solution");
        init = new emili::vrp::VrpSavingsInitialSolution(*instance);
    }

   else if (tm.checkToken(INITIAL_SA_VRP_PAR1))
   {
        printTab( "clarke and Wright savings parametrical 1 initial solution");
        init = new emili::vrp::VrpSavingsParam1InitialSolution(*instance);
   }

   else if (tm.checkToken(INITIAL_SA_VRP_PAR2))
   {
	float lambda = tm.getDecimal();
	float v = tm.getDecimal();
        printTab( "clarke and Wright savings parametrical 2 initial solution");
        init = new emili::vrp::VrpSavingsParam2InitialSolution(*instance,v,lambda);
   }
   else if (tm.checkToken(INITIAL_CHRISTOFIDES))
   {
        printTab( "Christofides initial solution");
        init = new emili::vrp::ChristofidesInitialSolution(*instance);
   }

    else if(tm.checkToken(INITIAL_LOCAL_SEARCH))
    {
        printTab( "Local search initial solution");
        emili::LocalSearch* lls = retrieveComponent(COMPONENT_ALGORITHM).get<emili::LocalSearch>();
        init = new emili::vrp::LocalSearchInitial(*instance, *lls);
    }
    prs::decrementTabLevel();

    return init;
}

//------------------------------neighborhoods------------------------------
emili::Neighborhood* prs::vrp::VrpBuilder::buildNeighborhood()
{
        prs::incrementTabLevel();
        emili::Neighborhood* neigh = nullptr;
        emili::vrp::VrpProblem* instance =(emili::vrp::VrpProblem*) gp.getInstance();

    if(tm.checkToken(NEIGHBORHOOD_INTER_ROUTE_SHIFT_1_0)) {
        printTab( "Neighborhood: Inter-route Shift(1,0)" );
         neigh = new emili::vrp::Shift_1_0_Neighborhood(*instance);
    }

    if(tm.checkToken(NEIGHBORHOOD_INTER_ROUTE_SHIFT_1_0_REPAIR)) {
         printTab( "Neighborhood: Inter-route Shift(1,0) Repair" );
        neigh = new emili::vrp::Shift_1_0_Neighborhood_With_Reparation(*instance);
    }

	else if (tm.checkToken(NEIGHBORHOOD_INTER_ROUTE_SHIFT_2_0)) {
            printTab( "Neighborhood: Inter-route Shift(2,0)" );
            neigh = new emili::vrp::Shift_2_0_Neighborhood(*instance);
    } 

    else if (tm.checkToken(NEIGHBORHOOD_INTER_ROUTE_SHIFT_2_0_REPAIR)) {
            printTab( "Neighborhood: Inter-route Shift(2,0) Repair" );
            neigh = new emili::vrp::Shift_2_0_Neighborhood_With_Reparation(*instance);
    } 

	else if(tm.checkToken( NEIGHBORHOOD_INTER_ROUTE_CROSS_EXCHANGE)){
            printTab( "Neighborhood: Inter-route Cross_Exchange" );
            neigh = new emili::vrp::Cross_Exchange_Neighborhood(*instance);
	} 

	else if (tm.checkToken(NEIGHBORHOOD_INTER_ROUTE_SWAP_1_1_REPAIR)) {
            printTab( "Neighborhood: Inter-route Swap(1,1) repair" );
            neigh = new emili::vrp::Swap_1_1_Neighborhood_With_Reparation(*instance);
    }

    else if (tm.checkToken(NEIGHBORHOOD_INTER_ROUTE_SWAP_1_1)) {
            printTab( "Neighborhood: Inter-route Swap(1,1)" );
            neigh = new emili::vrp::Swap_1_1_Neighborhood(*instance);
    }  

    else if (tm.checkToken(NEIGHBORHOOD_INTER_ROUTE_SWAP_2_1_REPAIR)) {
            printTab( "Neighborhood: Inter-route Swap(2,1)" );
            neigh = new emili::vrp::Swap_2_1_Neighborhood_With_Reparation(*instance);
    } 
    
	else if (tm.checkToken(NEIGHBORHOOD_INTER_ROUTE_SWAP_2_1)) {
            printTab( "Neighborhood: Inter-route Swap(2,1)" );
            neigh = new emili::vrp::Swap_2_1_Neighborhood(*instance);
    } 

	else if (tm.checkToken(NEIGHBORHOOD_INTER_ROUTE_SWAP_2_2)) {
            printTab( "Neighborhood: Inter-route Swap(2,2)" );
            neigh = new emili::vrp::Swap_2_2_Neighborhood(*instance);
    } 

    else if (tm.checkToken(NEIGHBORHOOD_INTER_ROUTE_SWAP_2_2_REPAIR)) {
            printTab( "Neighborhood: Inter-route Swap(2,2)" );
            neigh = new emili::vrp::Swap_2_2_Neighborhood_With_Reparation(*instance);
    } 

	else if (tm.checkToken(NEIGHBORHOOD_INTRA_ROUTE_REINSERTION)) {
            printTab( "Neighborhood: Intra-route Reinsertion" );
            neigh = new emili::vrp::Reinsertion_Neighborhood(*instance);
    } 

	else if (tm.checkToken(NEIGHBORHOOD_INTRA_ROUTE_EXCHANGE)) {
            printTab( "Neighborhood: Intra-route Reinsertion" );
            neigh = new emili::vrp::Exchange_Neighborhood(*instance);
    } 

	else if (tm.checkToken(NEIGHBORHOOD_INTRA_ROUTE_OR_OPT_2)) {
            printTab( "Neighborhood: Intra-route Reinsertion" );
            neigh = new emili::vrp::TwoOPT_Neighborhood(*instance);
    } 

    /*else if(tm.checkToken(INSERTION__REMOVAL_NEIGHBORHOOD)){
            printTab( "Insertion - Removal heuristics" );
            
    }*/

    prs::decrementTabLevel();
    return neigh;
}

//------------------------------ perturbation ------------------------------
emili::Perturbation* prs::vrp::VrpBuilder::buildPerturbation()
{
        prs::incrementTabLevel();
        emili::Perturbation* per = nullptr;
        std::ostringstream oss;
        
        if(tm.checkToken(PERTURBATION_RESTART)) {
            int n = tm.getInteger();
            oss.str(""); 
            oss  << "Restart perturbation, n =" << n << "";
            printTab(oss.str().c_str());
            emili::InitialSolution* init = retrieveComponent(COMPONENT_INITIAL_SOLUTION_GENERATOR).get<emili::InitialSolution>();
            per = new emili::vrp::RestartPerturbation(n,init);
        } else if(tm.checkToken(PERTURBATION_RESTART_LS)) {
            int n = tm.getInteger();
            oss.str(""); 
            oss  << "Restart perturbation, n =" << n << "";
            printTab(oss.str().c_str());
            emili::InitialSolution* init = retrieveComponent(COMPONENT_INITIAL_SOLUTION_GENERATOR).get<emili::InitialSolution>();
            emili::LocalSearch* ll = retrieveComponent(COMPONENT_ALGORITHM).get<emili::LocalSearch>();
            per = new emili::vrp::RestartPerturbation(n,init,ll);
        } else if(tm.checkToken(PERTURBATION_TABU)){
            emili::TabuMemory* tmem = retrieveComponent(COMPONENT_TABU_TENURE).get<emili::TabuMemory>();
            oss.str("");
            //oss  << "tabu perturbation, tenure size n =" << n << "";
            printTab(oss.str().c_str());
            per = new emili::vrp::TabuFitnessPerturbation(tmem);
	}



/*else if(tm.checkToken(PERTURBATION_MULTI_SWAP_1_1) ){
		emili::InitialSolution* init = retrieveComponent(COMPONENT_INITIAL_SOLUTION_GENERATOR).get<emili::InitialSolution>();
		emili::LocalSearch* ll = retrieveComponent(COMPONENT_ALGORITHM).get<emili::LocalSearch>();
		printTab("Perturbation multiSwap 11");
	} else if(tm.checkToken(PERTURBATION_MULTI_SHIFT_1_1 ) ){
		emili::InitialSolution* init = retrieveComponent(COMPONENT_INITIAL_SOLUTION_GENERATOR).get<emili::InitialSolution>();
		emili::LocalSearch* ll = retrieveComponent(COMPONENT_ALGORITHM).get<emili::LocalSearch>();
                printTab("Perturbation multiShift 1 1");
        }*/

        prs::decrementTabLevel();
        return per;
}

//--------------------------- acceptance criteria ------------------------------
emili::Acceptance* prs::vrp::VrpBuilder::buildAcceptance()
{
        prs::incrementTabLevel();
        emili::vrp::VrpProblem* instance =(emili::vrp::VrpProblem*) gp.getInstance();
        emili::Acceptance* acc = nullptr;
        std::ostringstream oss;
        if(tm.checkToken(ACCEPTANCE_SOA))
        {
            float n = tm.getDecimal();
            oss.str(""); 
            oss  << "soa metropolis like acceptance. temperature : "<<n;
            printTab(oss.str().c_str());
            acc = new  emili::vrp::SOAacceptance(n);
        }
        else if(tm.checkToken(ACCEPTANCE_TEST)) {
            int n = tm.getInteger();
            oss.str(""); 
            oss  << "Probabilistic Acceptance. improving solution accepted"<<n<<" % of the time";
            printTab(oss.str().c_str());
           acc = new  emili::vrp::TestAcceptance(*instance,n);
        }
    
        else if(tm.checkToken(ACCEPTANCE_SKEWED)) {
            float alpha = tm.getDecimal();
            int distance_method = tm.getInteger();
            oss.str(""); 
            oss  << "Skewed Acceptance";
            printTab(oss.str().c_str());
           acc = new  emili::vrp::SkewedAcceptanceDistance(*instance,alpha,distance_method);
        }
        prs::decrementTabLevel();
        return acc;
}

//--------------------------- termination ------------------------------

emili::Termination* prs::vrp::VrpBuilder::buildTermination()
{
    prs::incrementTabLevel();
    emili::vrp::VrpProblem* instance =(emili::vrp::VrpProblem*) gp.getInstance();
    emili::Termination* term = nullptr;
    if(tm.checkToken(TERMINATION_ITERA)) {
        int ti = tm.getInteger();        
        printTab("Relaxed local minima termination");
        std::ostringstream oss;
        oss << "number of max iterations "<< ti;
        printTabPlusOne(oss.str().c_str());
        term =  new emili::vrp::TerminationIterations(ti);
    }
    else if(tm.checkToken(TERMINATION_SOA)) {
        printTab("Max iteration number termination");
        int ti = instance->getNcustomers();
        ti = 2*(ti-1);
        std::ostringstream oss;
        oss << "number of max iterations "<< ti;
        printTabPlusOne(oss.str().c_str());
        term =  new emili::vrp::SOAtermination(ti);
    }




    prs::decrementTabLevel();
    return term;
}

//----------------------------Tabu List-----------------------------
emili::TabuMemory* prs::vrp::VrpBuilder::buildTabuTenure()
{
    prs::incrementTabLevel();
    std::ostringstream oss;
    emili::TabuMemory* tmem = nullptr;
    //printTab(oss.str().c_str());
    if(tm.checkToken(TABU_MEMORY_ALL_SOLUTION))
    {
        oss.str(""); oss << "USING ALL SOLUTION\n\t";
        int ts = tm.getInteger();
        oss << "Tabu tenure size " << ts;
        printTab(oss.str().c_str());
        tmem = new  emili::vrp::TabuAllSolution(ts);
    }
    else if(tm.checkToken(TABU_MEMORY_1_CAR))
    {
        oss.str(""); oss << "USING 1 CAR\n\t";
        int ts = tm.getInteger();
        oss << "Tabu tenure size " << ts;
        printTab(oss.str().c_str());
        tmem = new  emili::vrp::TabuOneCar(ts);
    }
    else if(tm.checkToken(TABU_ONE_POSITION))
    {
        oss.str(""); oss << "USING 1 POSITION \n\t";
        int ts = tm.getInteger();
        oss << "Tabu tenure size " << ts;
        printTab(oss.str().c_str());
        tmem = new  emili::vrp::TabuOnePosition(ts);
    }
    prs::decrementTabLevel();
    return tmem;
}

//------------------------INSERTION-------------------
emili::Insertion* prs::vrp::VrpBuilder::buildInsertion()
{
    emili::vrp::VrpProblem* instance = (emili::vrp::VrpProblem*) gp.getInstance();
    prs::incrementTabLevel();
    std::ostringstream oss;
    emili::Insertion* tmem = nullptr;
    printTab(oss.str().c_str());
    if(tm.checkToken(INSERTION_RANDOM_HEURISTIC))
    {
        oss.str(""); oss << "Insertion random heuristic\n\t";
        printTab(oss.str().c_str());
        emili::Removal* rev = buildRemoval();
        tmem = new  emili::vrp::Random_Insertion(*instance,rev);
    }

    prs::decrementTabLevel();
    return tmem;
}
//------------------------REMOVAL-------------------
emili::Removal* prs::vrp::VrpBuilder::buildRemoval()
{
    prs::incrementTabLevel();
    std::ostringstream oss;
    emili::Removal* tmem = nullptr;
    emili::vrp::VrpProblem* instance = (emili::vrp::VrpProblem*) gp.getInstance();
    printTab(oss.str().c_str());
    if(tm.checkToken(REMOVAL_RANDOM_HEURISTIC))
    {
        int n = tm.getInteger();
        oss.str(""); 
        oss.str(""); oss << "Removal random heuristic\n\t";
        printTab(oss.str().c_str());
        /*if(instance->problemSize < n){
            n = instance->problemSize;
        }*/
        tmem = new emili::vrp::Random_Removal(*instance,n);
    }

    prs::decrementTabLevel();
    return tmem;
}


/*--------------------------------------------------------------------------------------------------------*
 *-------------------------------------BOOLEANOS DE ACEPTACION--------------------------------------------*
 *--------------------------------------------------------------------------------------------------------*/

//comprueba que el problema ingresado puede ser usado por el parser
bool isParsable(std::string &problem)
{
    if(strcmp(problem.c_str(),PROBLEM_CAPACITATED_VRP)==0)
    {
        return true;
    }
    else
    {
        return false;
    }
}


/*
 *TO DO:
 *  Buscar porque ambas hacen lo mismo y donde son utilizadas/llamadas 
 */
bool prs::vrp::VrpBuilder::isCompatibleWith(char* problem_definition)
{
    std::string s(problem_definition);
    return isParsable(s);
}

bool prs::vrp::VrpBuilder::canOpenInstance(char* problem_definition)
{
    std::string s(problem_definition);
    return isParsable(s);
}

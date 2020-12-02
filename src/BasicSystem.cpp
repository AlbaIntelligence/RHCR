#include "BasicSystem.h"
#include <stdlib.h>
#include <boost/tokenizer.hpp>


BasicSystem::BasicSystem(const BasicGraph& G, MAPFSolver& solver):
    G(G), solver(solver), num_of_tasks(0), clustering(G, planning_window, planning_window) {}

BasicSystem::~BasicSystem() {}


// TODO: implement the random instance generator
/*bool BasicSystem::load_config(std::string fname)
{
    std::string line;
    std::ifstream myfile(fname.c_str());
    if (!myfile.is_open()) {
        std::cout << "Config file " << fname << " does not exist. " << std::endl;
        return false;
    }
    getline(myfile, line);
    boost::char_separator<char> sep(" ");
    boost::tokenizer<boost::char_separator<char> > tok(line, sep);
    boost::tokenizer<boost::char_separator<char> >::iterator beg = tok.begin();
    duration = atoi((*beg).c_str());
    getline(myfile, line);
    tok.assign(line, sep);
    fiducial = atoi((*beg).c_str());
    getline(myfile, line);
    tok.assign(line, sep);
    double length = atoi((*beg).c_str()) / fiducial;
    getline(myfile, line);
    tok.assign(line, sep);
    double width = atoi((*beg).c_str()) / fiducial;
    getline(myfile, line);
    tok.assign(line, sep);
    double v_max = atoi((*beg).c_str()) * duration / fiducial / 1000;
    getline(myfile, line);
    tok.assign(line, sep);
    double a_max = atoi((*beg).c_str()) * duration * duration / fiducial / 1000000;
    getline(myfile, line);
    tok.assign(line, sep);
    double rotate90 = atoi((*beg).c_str()) / duration;
    getline(myfile, line);
    tok.assign(line, sep);
    int rotate180 = atoi((*beg).c_str()) / duration;
    getline(myfile, line);
    tok.assign(line, sep);
    planning_window = atoi((*beg).c_str()) / duration;
    getline(myfile, line);
    tok.assign(line, sep);
    simulation_time = atoi((*beg).c_str()) / duration;

    myfile.close();
return true;

}
bool BasicSystem::generate_random_MAPF_instance()
{
    std::cout << "*** Generating instance " << seed << " ***" << std::endl;
    clock_t t = std::clock();
    // initialize_start_locations();
    // initialize_goal_locations();
    double runtime = (std::clock() - t) / CLOCKS_PER_SEC;
    std::cout << "Done! (" << runtime << " s)" << std::endl;
    // print_MAPF_instance();
    
    return true;
}


void BasicSystem::print_MAPF_instance() const
{
    std::cout << "*** instance " << seed << " ***" << std::endl;
    for (int k = 0; k < (int)starts.size(); k++)
    {
        cout << "Agent " << k << ": " << starts[k];
        for (int goal : goal_locations[k])
            cout << "->" << goal;
        cout << endl;
    }
}


void BasicSystem::save_MAPF_instance(std::string fname) const
{
    std::ofstream stats;
    stats.open(fname, std::ios::app);
    stats << starts.size();
    for (int k = 0; k < (int)starts.size(); k++)
    {
        stats << k << "," << starts[k].location;
        for (int goal : goal_locations[k])
            cout << "," << goal;
        cout << endl;
    }
    stats.close();
}

bool BasicSystem::read_MAPF_instance(std::string fname)
{
    std::string line;
    std::ifstream myfile(fname.c_str());
    if (!myfile.is_open()) {
        std::cout << "MAPF instance file " << fname << " does not exist. " << std::endl;
        return false;
    }

    std::cout << "*** Reading instance " << fname << " ***" << std::endl;

    getline(myfile, line);
    boost::char_separator<char> sep(",");
    boost::tokenizer<boost::char_separator<char> > tok(line, sep);
    boost::tokenizer<boost::char_separator<char> >::iterator beg = tok.begin();
    this->num_of_drives = atoi((*beg).c_str()); // read number of cols
    this->starts.resize(num_of_drives);
    this->goal_locations.resize(num_of_drives);

    for (int i = 0; i < num_of_drives; i++) {
        getline(myfile, line);
        boost::tokenizer<boost::char_separator<char> > tok(line, sep);
        beg = tok.begin();
        beg++; // skip id
        starts[i] = State(std::atoi(beg->c_str()));
        goal_locations[i].emplace_back(std::atoi(beg->c_str()));
    }

    myfile.close();
    return true;
}


bool BasicSystem::run()
{
	bool sol = pbs.run(starts, goal_locations, time_limit, vector<Path>(), PriorityGraph());
	pbs.save_results(outfile, std::to_string(num_of_drives) + "," + std::to_string(seed));
	pbs.best_node->priorities.save_as_digraph("goal_node.gv");
	return sol;
}
*/

bool BasicSystem::load_locations()
{
	string fname = G.map_name + "_rotation=" + std::to_string(consider_rotation) +
		"_" + std::to_string(num_of_drives) + ".agents";
    std::ifstream myfile (fname.c_str());
    if (!myfile.is_open())
		return false;

    string line;
    getline (myfile,line);
    boost::char_separator<char> sep(",");

    if (atoi(line.c_str()) != num_of_drives)
    {
        cout << "The agent file does not match the settings." << endl;
        exit(-1);
    }
    for (int k = 0; k < num_of_drives; k++)
    {
        getline (myfile, line);
        boost::tokenizer< boost::char_separator<char> > tok(line, sep);
        boost::tokenizer< boost::char_separator<char> >::iterator beg=tok.begin();
        // starts
        int start_loc = atoi((*beg).c_str());
        beg++;
        int start_orient = atoi((*beg).c_str());
        beg++;
        starts[k] = State(start_loc, 0, start_orient);
        paths[k].emplace_back(starts[k]);
        finished_tasks[k].push_back(std::make_pair(start_loc, 0));
        // goals
        int goal = atoi((*beg).c_str());
        goal_locations[k].emplace_back(goal, 0);
    }
    myfile.close();
	return true;
}


void BasicSystem::update_start_locations()
{
    for (int k = 0; k < num_of_drives; k++)
    {
        starts[k] = State(paths[k][timestep].location, 0, paths[k][timestep].orientation);
    }
}


void BasicSystem::update_paths(const std::vector<Path*>& MAPF_paths, int max_timestep = INT_MAX)
{
    for (int k = 0; k < num_of_drives; k++)
    {
        int length = min(max_timestep, (int) MAPF_paths[k]->size());
        paths[k].resize(timestep + length);
        for (int t = 0; t < length; t++)
        {
            if (MAPF_paths[k]->at(t).location < 0)
                paths[k][timestep + t] = State(-starts[k].location - 1, timestep + t, starts[k].orientation);
            else
            {
                paths[k][timestep + t] = MAPF_paths[k]->at(t);
                paths[k][timestep + t].timestep = timestep + t;
            }
        }
    }
}

void BasicSystem::update_paths(const std::vector<Path>& MAPF_paths, int max_timestep = INT_MAX)
{
    for (int k = 0; k < num_of_drives; k++)
    {
        int length = min(max_timestep, (int) MAPF_paths[k].size());
        paths[k].resize(timestep + length);
        for (int t = 0; t < length; t++)
        {
            paths[k][timestep + t] = MAPF_paths[k][t];
            paths[k][timestep + t].timestep = timestep + t;
        }
    }
}

void BasicSystem::update_initial_paths(vector<Path>& initial_paths) const
{
    initial_paths.clear();
    initial_paths.resize(num_of_drives);
    for (int k = 0; k < num_of_drives; k++)
    {
        // check whether the path traverse every goal locations
        int i = (int)goal_locations[k].size() - 1;
        int j = (int)paths[k].size() - 1;
        while (i >= 0 && j >= 0)
        {
            while (j >= 0 && paths[k][j].location != goal_locations[k][i].first &&
			paths[k][j].timestep >= goal_locations[k][i].second)
                j--;
            i--;
        }
        if (j < 0)
            continue;
          
        if ((int) paths[k].size() <= timestep + planning_window)
            continue;

        initial_paths[k].resize(paths[k].size() - timestep);
        for (int t = 0; t < (int)initial_paths[k].size(); t++)
        {
            initial_paths[k][t] = paths[k][timestep + t];
            initial_paths[k][t].timestep = t;
        }
    }
}

void BasicSystem::update_initial_constraints(list< tuple<int, int, int> >& initial_constraints) const
{
    initial_constraints.clear();
    for (int k = 0; k < num_of_drives; k++)
    {
        int prev_location = -1;
        for (int t = timestep; t > max(0, timestep - k_robust); t--)
        {
            int curr_location = paths[k][t].location;
            if (curr_location < 0)
                continue;
            else if (curr_location != prev_location)
            {
                initial_constraints.emplace_back(k, curr_location, t + k_robust + 1 - timestep);
                prev_location = curr_location;
            }
        }
    }
}



bool BasicSystem::check_collisions(const vector<Path>& input_paths) const
{
	for (int a1 = 0; a1 < (int)input_paths.size(); a1++)
	{
		for (int a2 = a1 + 1; a2 < (int)input_paths.size(); a2++)
		{
			// TODO: add k-robust
			size_t min_path_length = input_paths[a1].size() < input_paths[a2].size() ? input_paths[a1].size() : input_paths[a2].size();
			for (size_t timestep = 0; timestep < min_path_length; timestep++)
			{
				int loc1 = input_paths[a1].at(timestep).location;
				int loc2 = input_paths[a2].at(timestep).location;
				if (loc1 == loc2)
					return true;
				else if (timestep < min_path_length - 1
					&& loc1 == input_paths[a2].at(timestep + 1).location
					&& loc2 == input_paths[a1].at(timestep + 1).location)
					return true;
			}
			if ((hold_endpoints || useDummyPaths) && input_paths[a1].size() != input_paths[a2].size())
			{
				int a1_ = input_paths[a1].size() < input_paths[a2].size() ? a1 : a2;
				int a2_ = input_paths[a1].size() < input_paths[a2].size() ? a2 : a1;
				int loc1 = input_paths[a1_].back().location;
				for (size_t timestep = min_path_length; timestep < input_paths[a2_].size(); timestep++)
				{
					int loc2 = input_paths[a2_].at(timestep).location;
					if (loc1 == loc2)
						return true;
				}
			}
		}
	}
	return false;
}

bool BasicSystem::congested() const
{
	if (simulation_window <= 1)
		return false;
    int wait_agents = 0;
    for (const auto& path : paths)
    {
        int t = 0;
        while (t < simulation_window && path[timestep].location == path[timestep + t].location &&
                path[timestep].orientation == path[timestep + t].orientation)
            t++;
        if (t == simulation_window)
            wait_agents++;
    }
    return wait_agents > num_of_drives / 2;  // more than half of drives didn't make progress
}

// move all agents from start_timestep to end_timestep
// return a list of finished tasks
list<tuple<int, int, int>> BasicSystem::move()
{
    int start_timestep = timestep;
    int end_timestep = timestep + simulation_window;

	list<tuple<int, int, int>> finished_tasks; // <agent_id, location, timestep>

    for (int t = start_timestep; t <= end_timestep; t++)
    {
        for (int k = 0; k < num_of_drives; k++) {
            // Agents waits at its current locations if no future paths are assigned
            while ((int) paths[k].size() <= t)
            { // This should not happen?
                State final_state = paths[k].back();
                paths[k].emplace_back(final_state.location, final_state.timestep + 1, final_state.orientation);
            }
        }
    }


    for (int t = start_timestep; t <= end_timestep; t++)
    {
        for (int k = 0; k < num_of_drives; k++)
        {
            State curr = paths[k][t];

            /* int wait_times = 0; // wait time at the current location
            while (wait_times < t && paths[k][t - wait_times] != curr)
            {
                wait_times++;
            }*/

            // remove goals if necessary
            if ((!hold_endpoints || paths[k].size() == t + 1) && !goal_locations[k].empty() && 
				curr.location == goal_locations[k].front().first &&
				curr.timestep >= goal_locations[k].front().second) // the agent finish its current task
            {
                goal_locations[k].erase(goal_locations[k].begin());
				finished_tasks.emplace_back(k, curr.location, t);
            }

            // check whether the move is valid
            if (t > 0)
            {
                State prev = paths[k][t - 1];

                if (curr.location == prev.location)
                {
                    if (G.get_rotate_degree(prev.orientation, curr.orientation) == 2)
                    {
                        cout << "Drive " << k << " rotates 180 degrees from " << prev << " to " << curr << endl;
                        save_results();
                        exit(-1);
                    }
                }
                else if (consider_rotation)
                {
                    if (prev.orientation != curr.orientation)
					{
						cout << "Drive " << k << " rotates while moving from " << prev << " to " << curr << endl;
						save_results();
						exit(-1);
					}
					else if ( !G.valid_move(prev.location, prev.orientation) ||
                        prev.location + G.move[prev.orientation] != curr.location)
                    {
                        cout << "Drive " << k << " jump from " << prev << " to " << curr << endl;
                        save_results();
                        exit(-1);
                    }
                }
				else
				{
					int dir = G.get_direction(prev.location, curr.location);
					if (dir < 0 || !G.valid_move(prev.location, dir))
					{
						cout << "Drive " << k << " jump from " << prev << " to " << curr << endl;
						save_results();
						exit(-1);
					}
				}
            }

            // Check whether this move has conflicts with other agents
			if (G.types[curr.location] != "Magic")
			{
				for (int j = k + 1; j < num_of_drives; j++)
				{
					for (int i = max(0, t - k_robust); i <= min(t + k_robust, end_timestep); i++)
					{
						if ((int)paths[j].size() <= i)
							break;
						if (paths[j][i].location == curr.location)
						{
							cout << "Drive " << k << " at " << curr << " has a conflict with drive " << j
								<< " at " << paths[j][i] << endl;
							save_results(); //TODO: write termination reason to files
							exit(-1);
						}
					}
				}
			}
        }
    }
    return finished_tasks;
}


void BasicSystem::add_partial_priorities(const vector<Path>& initial_paths, PriorityGraph& initial_priorities) const
{
    list<int> low_priorities;
    list<int> high_priorities;
    for (int k = 0; k < num_of_drives; k++)
    {
        if (initial_paths[k].empty())
            low_priorities.push_back(k);
        else
            high_priorities.push_back(k);
    }

    for (auto low : low_priorities)
    {
        for (auto high : high_priorities)
            initial_priorities.add(low, high);
    }
}

void BasicSystem::save_results()
{
	if (screen)
		std::cout << "*** Saving " << seed << " ***" << std::endl;
    clock_t t = std::clock();
    std::ofstream output;

    // settings
    output.open(outfile + "/config.txt", std::ios::out);
    output << "map: " << G.map_name << std::endl
        << "#drives: " << num_of_drives << std::endl
        << "seed: " << seed << std::endl
        << "solver: " << solver.get_name() << std::endl
        << "time_limit: " << time_limit << std::endl
        << "simulation_window: " << simulation_window << std::endl
        << "planning_window: " << planning_window << std::endl
        << "simulation_time: " << simulation_time << std::endl
        << "robust: " << k_robust << std::endl
        << "rotate: " << consider_rotation << std::endl
        << "use_dummy_paths: " << useDummyPaths << std::endl
        << "hold_endpoints: " << hold_endpoints << std::endl;

    output.close();

    // tasks
    output.open(outfile + "/tasks.txt", std::ios::out);
    output << num_of_drives << std::endl;
    for (int k = 0; k < num_of_drives; k++)
    {
        int prev = finished_tasks[k].front().first;
        for (auto task : finished_tasks[k])
        {
            output << task.first << "," << task.second << ",";
            if (task.second != 0)
                output << G.heuristics.at(task.first)[prev];
            output << ";";
            prev = task.first;
        }
        for (auto goal : goal_locations[k]) // tasks that have not been finished yet
        {
            output << goal.first << ",-1,;";
        }
        output << std::endl;
    }
    output.close();

    // paths
    output.open(outfile + "/paths.txt", std::ios::out);
    output << num_of_drives << std::endl;
    for (int k = 0; k < num_of_drives; k++)
    {
        for (const auto& p : paths[k])
        {
            if (p.timestep <= timestep)
                output << p << ";";
        }
        output << std::endl;
    }
    output.close();
    saving_time = (double)(std::clock() - t) / CLOCKS_PER_SEC;
	if (screen)
		std::cout << "Done! (" << saving_time << " s)" << std::endl;
}


void BasicSystem::update_travel_times(unordered_map<int, double>& travel_times)
{
    if (travel_time_window <= 0)
        return;

    travel_times.clear();
    unordered_map<int, int> count;

    int t_min = max(0, timestep - travel_time_window);
    if (t_min >= timestep)
        return;
    for (auto path : paths)
    {
        int t = timestep;
        while (t >= t_min)
        {
            int loc = path[t].location;
            int dir = path[t].orientation;
            int wait = 0;
            while (t > wait && path[t - 1 - wait].location == loc && path[t - 1 - wait].orientation == dir)
                wait++;
            auto it = travel_times.find(loc);
            if (it == travel_times.end())
            {
                travel_times[loc] = wait;
                count[loc] = 1;
            }
            else
            {
                travel_times[loc] += wait;
                count[loc] += 1;
            }
            t = t - 1 - wait;
        }
    }

    for (auto it : count)
    {
        if (it.second > 1)
        {
            travel_times[it.first] /= it.second;
        }
    }
}


void BasicSystem::solve()
{
	LRAStar lra(G, solver.path_planner);
	lra.simulation_window = simulation_window;
	lra.k_robust = k_robust;
	solver.clear();
	if (solver.get_name() == "LRA")
	{
		// predict travel time
		unordered_map<int, double> travel_times;
		update_travel_times(solver.travel_times);

		bool sol = solver.run(starts, goal_locations, time_limit);
		update_paths(solver.solution);
	}
	else if (solver.get_name() == "WHCA")
	{
		update_initial_constraints(solver.initial_constraints);

		bool sol = solver.run(starts, goal_locations, time_limit);
		if (sol)
		{
			update_paths(solver.solution);
		}
		else
		{
			lra.resolve_conflicts(solver.solution);
			update_paths(lra.solution);
		}
	}
    else // PBS or ECBS
    {
     //PriorityGraph initial_priorities;
     update_initial_constraints(solver.initial_constraints);

     // solve
     if (hold_endpoints || useDummyPaths)
     {
         vector<State> new_starts;
         vector< vector<pair<int, int> > > new_goal_locations;
         for (int i : new_agents)
         {
             new_starts.emplace_back(starts[i]);
             new_goal_locations.emplace_back(goal_locations[i]);
         }
         vector<Path> planned_paths(num_of_drives);
         solver.initial_rt.clear();
         auto p = new_agents.begin();
         for (int i = 0; i < num_of_drives; i++)
         {
             if (p == new_agents.end() || *p != i)
             {
                 planned_paths[i].resize(paths[i].size() - timestep);
                 for (int t = 0; t < (int)planned_paths[i].size(); t++)
                 {
                     planned_paths[i][t] = paths[i][timestep + t];
                     planned_paths[i][t].timestep = t;
                 }
                 solver.initial_rt.insertPath2CT(planned_paths[i]);
             }
             else
                 ++p;
         }
         if (!new_agents.empty())
         {
             bool sol = solver.run(new_starts, new_goal_locations, time_limit);
             if (sol)
             {
                 auto pt = solver.solution.begin();
                 for (int i : new_agents)
                 {
                     planned_paths[i] = *pt;
                     ++pt;
                 }
                 if (check_collisions(planned_paths))
                 {
                     cout << "COLLISIONS!" << endl;
                     exit(-1);
                 }
             }
             else
             {
                 if (timestep == 0)
                        sol = solve_by_WHCA(planned_paths, new_starts, new_goal_locations);
                if (!sol)
                {
                    lra.resolve_conflicts(solver.solution);
                    auto pt = lra.solution.begin();
                    for (int i : new_agents)
                    {
                        planned_paths[i] = *pt;
                        ++pt;
                    }
                }
             }
         }
         // lra.resolve_conflicts(planned_paths, k_robust);
         update_paths(planned_paths);
     }
     else
     {
         bool sol = solver.run(starts, goal_locations, time_limit);
         if (sol)
         {
             if (log)
                 solver.save_constraints_in_goal_node(outfile + "/goal_nodes/" + std::to_string(timestep) + ".gv");
             update_paths(solver.solution);
         }
         else
         {
             lra.resolve_conflicts(solver.solution);
             update_paths(lra.solution);
         }
     }
     if (log)
         solver.save_search_tree(outfile + "/search_trees/" + std::to_string(timestep) + ".gv");

    }
    solver.save_results(outfile + "/solver.csv", std::to_string(timestep) + ","
                                    + std::to_string(num_of_drives) + "," + std::to_string(seed));
}


void BasicSystem::solve_by_groups()
{
    LRAStar lra(G, solver.path_planner);
    lra.simulation_window = simulation_window;
    lra.k_robust = k_robust;
    solver.clear();
    assert(solver.get_name() == "PBS" || solver.get_name() == "ECBS");
    assert(!hold_endpoints && ! useDummyPaths);
    //PriorityGraph initial_priorities;
    update_initial_constraints(solver.initial_constraints);
    // clustering
    clustering.clear();
    clustering.updateLocations(starts, goal_locations);
    // clustering.writeDistanceMatrixToFile();
    clustering.run();
    vector<Path> planned_paths(num_of_drives);
    double runtime = 0;
    int numofClusters = 0;
    PriorityGraph copyPriorities;

    //Plan path for groups
    for (const auto& group : clustering.getClusters())
    {
        cout << group.size() << " agents" << endl;
        vector<State> group_starts(group.size());
        vector< vector<pair<int, int> > > group_goal_locations(group.size());
        solver.indexes.clear();
        solver.indexes.resize(group.size());

        for (int i = 0; i < group.size(); i++){
            group_starts[i] = starts[group[i]];
            group_goal_locations[i] = goal_locations[group[i]];
            solver.indexes[i] = group[i];
        }
        bool sol = solver.run(group_starts, group_goal_locations, time_limit);
        auto pt = solver.solution.begin();
        for (const int i : group)
        {
            planned_paths[i] = *pt;
            /*if (planned_paths[i][0] != starts[i]) {
                cout << "ERROR: Initial paths don't line up with start point" << "\n";
            }
            if (planned_paths[i][planned_paths[i].size()-1].location != goal_locations[i][goal_locations[i].size()-1].first) {
                cout << "ERROR: Initial goals don't line up with goal point" << "\n";
            }
            std::cerr << "Start " << i << " " << starts[i] << std::endl;
            std::cerr << "Path " << i << " " << planned_paths[i];
            for (auto g : goal_locations[i]) {
                std::cerr << "Goal " << i << "first :" << g.first << "second: " << g.second << std::endl;
            }
            std::cerr << std::endl;*/
            
            ++pt;
        }
        solver.best_node->priorities.setNumofAgents(group.size());
        //Merge Priorities G together
        
        for (auto g : solver.best_node->priorities.G) {
            std::cerr << "First: " << g.first << "\n";
            std::cerr << "Set Values: ";
            for (auto itr = g.second.begin(); itr != g.second.end(); itr++)
                std::cerr << (*itr) << " ";
            std::cerr << "\n";
        }
        copyPriorities.addTogether(solver.best_node->priorities, solver.indexes);
        runtime += solver.runtime;
        ++numofClusters;
    }
    //dummy_start is root node
    //PBS -> Initial Paths 

    vector<State> new_starts(num_of_drives);
    vector<vector<pair<int, int>>> new_goals(num_of_drives);
    vector<int> orders(num_of_drives);
    solver.initial_paths.resize(num_of_drives);
    solver.initial_priorities.copy(copyPriorities);
    int numIndex = 0;

    for (const auto& group : clustering.getClusters()){
        for (int i = 0; i < group.size(); i++) {
            new_starts[numIndex] = starts[group[i]];
            solver.initial_paths[numIndex] = planned_paths[group[i]];
            new_goals[numIndex] = goal_locations[group[i]];
            orders[numIndex] = group[i];
            ++numIndex;
        }
    }
    /*for (int i = 0; i < num_of_drives; ++i){
        if (solver.initial_paths[i][0] != new_starts[i]) {
            cout << "ERROR: Initial paths don't line up with start point" << "\n";
        }
        if (new_goals[i][new_goals[i].size()-1].first != solver.initial_paths[i][solver.initial_paths[i].size() - 1].location) {
            std::cerr << "ERROR: Initial goals don't line up with goal point" << "\n";
        }*/
        /*std::cerr << "Start " << i << " " << new_starts[i] << std::endl;
        std::cerr << "Path " << i << " " << solver.initial_paths[i] << std::endl;
        for (auto g : new_goals[i]) {
            std::cerr << "Goal " << i << "first :" << g.first << "second: " << g.second << std::endl;
        }
        std::cerr<<std::endl;
    }*/

    //Uses planned_path in the group_starts, in the root node
    bool sol = solver.run(new_starts, new_goals, time_limit);
    auto pt = solver.solution.begin();
    for (int i = 0; i < num_of_drives; ++i){
        planned_paths[i] = *pt;
        ++pt;
        solver.initial_paths[i] = planned_paths[i];
        
    }
    runtime += solver.runtime;

    //Check for Collisions
    assert(k_robust == 0);
    int num_of_collsions = 0;
    for (int i = 0; i < num_of_drives; i++)
    {
        for (int j = i + 1; j < num_of_drives; j++)
        {
            int size = min((int)min(planned_paths[i].size(), planned_paths[j].size()), planning_window);
            for (int t = 0; t < size; t++)
            {
                int loc1 = planned_paths[i][t].location;
                int loc2 = planned_paths[j][t].location;
                if (loc1 == loc2 ||
                    (timestep < size - 1
                         && loc1 == planned_paths[j][t + 1].location
                         && loc2 == planned_paths[i][t + 1].location))
                {
                    num_of_collsions++;
                    break;
                }
            }
        }
    }
    cout << num_of_collsions << " collisions" << endl;

    lra.resolve_conflicts(planned_paths);
    update_paths(lra.solution);

    std::ofstream stats;
    stats.open(outfile + "/collisions.csv", std::ios::app);
    stats << runtime << "," << num_of_collsions << ","
            << clustering.getClusters().front().size() << ","
            << num_of_drives << endl;
    stats.close();

    if (log)
        solver.save_search_tree(outfile + "/search_trees/" + std::to_string(timestep) + ".gv");

    solver.save_results(outfile + "/solver.csv", std::to_string(timestep) + ","
                                                 + std::to_string(num_of_drives) + "," + std::to_string(seed));
}



bool BasicSystem::solve_by_WHCA(vector<Path>& planned_paths,
	const vector<State>& new_starts, const vector< vector<pair<int, int> > >& new_goal_locations)
{
	WHCAStar whca(G, solver.path_planner);
	whca.initial_rt.hold_endpoints = true;
	whca.initial_rt.map_size = G.size();
	whca.initial_rt.k_robust = k_robust;
	whca.initial_rt.window = INT_MAX;
	whca.initial_rt.copy(solver.initial_rt);
	bool sol = false;
	sol = whca.run(new_starts, new_goal_locations, time_limit);
	whca.save_results(outfile + "/solver.csv", std::to_string(timestep) + ","
		+ std::to_string(num_of_drives) + "," + std::to_string(seed));
	if (sol)
	{
		auto pt = whca.solution.begin();
		for (int i : new_agents)
		{
			planned_paths[i] = *pt;
			++pt;
		}
	}
	whca.clear();
	return sol;
}

void BasicSystem::initialize_solvers()
{
	solver.k_robust = k_robust;
	solver.window = planning_window;
	solver.hold_endpoints = hold_endpoints || useDummyPaths;
	solver.screen = screen;

	solver.initial_rt.hold_endpoints = true;
	solver.initial_rt.map_size = G.size();
	solver.initial_rt.k_robust = k_robust;
	solver.initial_rt.window = INT_MAX;
}

bool BasicSystem::load_records()
{
	boost::char_separator<char> sep1(";");
	boost::char_separator<char> sep2(",");
	string line;

	// load paths
	std::ifstream myfile(outfile + "/paths.txt");

	if (!myfile.is_open())
			return false;

	timestep = INT_MAX;
	getline(myfile, line);
	if (atoi(line.c_str()) != num_of_drives)
	{
		cout << "The path file does not match the settings." << endl;
		exit(-1);
	}
	for (int k = 0; k < num_of_drives; k++)
	{
		getline(myfile, line);
		boost::tokenizer< boost::char_separator<char> > tok1(line, sep1);
		for (auto task : tok1)
		{
			boost::tokenizer< boost::char_separator<char> > tok2(task, sep2);
			boost::tokenizer< boost::char_separator<char> >::iterator beg = tok2.begin();
			int loc = atoi((*beg).c_str());
			beg++;
			int orientation = atoi((*beg).c_str());
			beg++;
			int time = atoi((*beg).c_str());
			paths[k].emplace_back(loc, time, orientation);
		}
		timestep = min(timestep, paths[k].back().timestep);
	}
	myfile.close();

	// pick the timestep
	timestep = int((timestep - 1) / simulation_window) * simulation_window; //int((timestep - 1) / simulation_window) * simulation_window;

	// load tasks
	myfile.open(outfile + "/tasks.txt");
	if (!myfile.is_open())
		return false;

	getline(myfile, line);
	if (atoi(line.c_str()) != num_of_drives)
	{
		cout << "The task file does not match the settings." << endl;
		exit(-1);
	}
	for (int k = 0; k < num_of_drives; k++)
	{
		getline(myfile, line);
		boost::tokenizer< boost::char_separator<char> > tok1(line, sep1);
		for (auto task : tok1)
		{
			boost::tokenizer< boost::char_separator<char> > tok2(task, sep2);
			boost::tokenizer< boost::char_separator<char> >::iterator beg = tok2.begin();
			int loc = atoi((*beg).c_str());
			beg++;
			int time = atoi((*beg).c_str());
			if (time >= 0 && time <= timestep)
			{
				finished_tasks[k].emplace_back(loc, time);
				timestep = max(timestep, time);
			}
			else
			{
				goal_locations[k].emplace_back(loc, 0);
			}
		}
	}
	myfile.close();
	return true;
}


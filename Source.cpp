// E2b
// Guillaume Lethuillier
// au 31 mai 2015

#include "Header.h"

int main()
{
	unsigned long long int compteur;

	int population = POPULATION;

	int i, j;
	int nb_lignes = (int)sqrt(PIECES);

	int niveau;
	int nb_solutions = 0;

	std::string Chemin;

	#ifdef OS_WIN
	Chemin = "C:/Users/Guillaume/Dropbox/Partage/";
	#else
	Chemin = "/Users/lethuillierd/Dropbox/Partage/";
	#endif

	// log $
	std::string TXT_log = Chemin + "Logs/log_" + std::to_string(PIECES) + ".txt";
	std::ofstream log(TXT_log, std::ios_base::app | std::ios_base::out);
	log << "#" << std::endl;

	// 1| OUVERTURE PUZZLE_.TXT
	std::vector<int> R_, A_, B_, C_, D_;

	if (!(fichier_ouverture(Chemin, R_, A_, B_, C_, D_)))
	{
		std::cout << "!ERR : fichier PUZZLE non ouvert\n";
		return 0;
	}

	if (PIECES == 256 && TEST != 1)
		if (!(fichier_hash(R_, A_, B_, C_, D_)))
		{
			std::cout << "!ERR : fichier PUZZLE non ouvert\n";
			return 0;
		}
		
	std::vector<int> R, A, B, C, D;
	R = R_;  A = A_; B = B_; C = C_, D = D_;
	

	bool fin_recherche = false;
	Noeud * _emplacements = new Noeud[PIECES];

	std::vector<int> rotations;

	Individu * _rotations = new Individu[population];

	std::uniform_int_distribution<> aleADN(0, 3);
	std::uniform_int_distribution<> mutation(0, PIECES-1);
	
	for (i = 0; i < population; i++)
		_rotations[i].ADN.resize(PIECES);

	std::unordered_map<int, double> classement_individus;

	std::vector<int> retenus;
	std::vector<double> classement_fitness;
	classement_fitness.resize(population);

	int best;
	double fitness_ref;

	
	do
	{
	Reset: 
	
		// Nouveaux individus
		for (i = 0; i < population; ++i)
		{
			for (j = 0; j < PIECES; ++j)
				_rotations[i].ADN[j] = aleADN(gen);
		}

		fitness_ref = 0.0;

		for (i = 0; i < population; ++i)
			_rotations[i].fitness = 0.0;

		compteur = 0;


/*
//  ROTATIONS
// -----------
*/


		bool fin_rotations = false;
		
		do
		{

			for (i = 0; i < population; ++i)
			{
				// Affichage
				if (_rotations[i].fitness > fitness_ref)
				{
					fitness_ref = _rotations[i].fitness;
					if (fitness_ref <0.3)
						std::cout << ".";
					else
						std::cout << "x";
				}
		
				// Rotations
				rotation_pieces(A_, B_, C_, D_, A, B, C, D, _rotations[i].ADN);

				// Evaluation
				evaluation_individu(A, B, C, D, _rotations[i].fitness);

				if (_rotations[i].fitness == 1)
				{
					rotations = _rotations[i].ADN;
					fin_rotations = true;
					break;
				}

				// Classement
				classement_fitness[i] = _rotations[i].fitness;
				classement_individus[i] = _rotations[i].fitness;
			}

			// Best
			sort(classement_fitness.begin(), classement_fitness.end(), std::greater<double>());

			for (auto it = classement_individus.begin(); it != classement_individus.end(); ++it)
				if (it->second == classement_fitness[0])
					best = it->first;

			std::vector<int> ADN_best;
			ADN_best.clear();

			ADN_best = _rotations[best].ADN;

			// roulette
			double fitness_total=0.0;

			for (i = 0; i < population; ++i)
				fitness_total += _rotations[i].fitness;

			std::uniform_real_distribution<> roulette(0, fitness_total);

			retenus.clear();

			do{
				double r = roulette(gen);
				j = 0;

				while (r > 0)
				{
					r -= _rotations[j].fitness;
					++j;
				}
				retenus.push_back(j-1);
			} while (retenus.size() < population *2);
			
			
			_rotations[1].ADN = ADN_best; // copie best soumis à mutation

			if (fin_rotations != true)
			{
				for (i = 0; i < population; ++i)
				{
					// Reproduction
					reproduction(_rotations[retenus[i]].ADN, _rotations[retenus[i+population]].ADN);
					
					// Mutation
					if (mutation(gen) <= MUTATION)
						for (j = 0; j < PIECES; ++j) 
							_rotations[i].ADN[j] = aleADN(gen);

					for (j = 0; j < PIECES; ++j)
						if (mutation(gen) <= MUTATION)
						_rotations[i].ADN[j] = aleADN(gen);
				}
			}

			_rotations[0].ADN = ADN_best; // preservation best

			++compteur;
			if (compteur > COMPTEUR)
			{
				std::cout << "\n";
				goto Reset;
			}

		} while (!(fin_rotations));


/*
//  EMPLACEMENTS
// --------------
*/


		// Sélection
		if (VERBOSE == 1)
		{
			std::cout << "\n # Selection\n";
			for (i = 0; i < PIECES; ++i) 
				std::cout << rotations[i];
			std::cout << "\n";
		}
		std::cout << " # RECHERCHE EMPLACEMENTS \n";

		// *| ARBRE

		bool fin_emplacements = false;


		// Initialisation
		int niveau_ref = 0;
		int intermediaire = 0;

		std::vector<int> restants;

		for (i = 0; i < PIECES; ++i)
			restants.push_back(i);

		std::vector<int> progression;

		niveau = 0;

		int descente_max = 0;

		// Id. & sélection racine
		for (i = 0; i < PIECES; ++i)
			if (A[i] == 0 && D[i] == 0)
			{
				progression.push_back(i);
				break;
			}

		if (progression.size() == 0)
			goto Reset;

		restants.erase(remove(restants.begin(), restants.end(), progression[0]), restants.end());
		restants.shrink_to_fit();

		// Noeuds suivants
		liste_noeuds
			(
			progression
			, _emplacements[niveau + 1].branche
			, restants
			, A
			, B
			, C
			, D
			, nb_lignes
			, niveau
			);
		
		filtrage_noeuds
			(
			_emplacements[niveau + 1].branche
			, restants
			, A
			, B
			, C
			, D
			, niveau
			);
			
		++niveau;

		// Construction & parcours de l'arbre
		do
		{

			if (niveau == 0)
			{
				std::cout << "\n # ARBRE PARCOURU\n";
				log << "-------------" << std::endl;
				log << "Population : " << population << std::endl;
				log << "Mutation   : " << MUTATION << std::endl;
				log << "Compteur   : " << compteur << std::endl;
				log << "Descente   : " << descente_max << std::endl;
				fin_emplacements = true; break;
			}

			if (_emplacements[niveau].branche.size() > 0)
			{
				progression.push_back(_emplacements[niveau].branche[0]);
				_emplacements[niveau].branche.erase(_emplacements[niveau].branche.begin());
				_emplacements[niveau].branche.shrink_to_fit();
				restants.erase(remove(restants.begin(), restants.end(), progression[niveau]), restants.end());
				restants.shrink_to_fit();

				// Noeud suivant
				liste_noeuds
					(
					progression
					, _emplacements[niveau + 1].branche
					, restants
					, A
					, B
					, C
					, D
					, nb_lignes
					, niveau
					);
				
				filtrage_noeuds
					(
					_emplacements[niveau + 1].branche
					, restants
					, A
					, B
					, C
					, D
					, niveau
					);
				
				++niveau;
				 std::cout << "+";
			}
			else if (_emplacements[niveau].branche.size() == 0)
			{
		Poursuite_exploration:

				restants.push_back(progression[niveau - 1]);
				progression.erase(progression.begin() + (niveau - 1));
				progression.shrink_to_fit();
				_emplacements[niveau].branche.clear();
				_emplacements[niveau].branche.shrink_to_fit();
				--niveau;
				 std::cout << "-";
			}


			if (niveau_ref < niveau)
			{
				affichage_progression(niveau, niveau_ref, progression, R);
				descente_max = niveau_ref;
			}

			if (niveau == PIECES - 1)
			{
				niveau_ref = 0;

				std::vector<int> solution = progression;
				solution.push_back(restants[0]);

				controle_solution
					(
					solution
					, R
					, A
					, B
					, C
					, D
					, Chemin
					, nb_solutions
					, rotations
					);

				solution.clear();
				goto Poursuite_exploration;
			}
		} while (!(fin_emplacements));

		if (nb_solutions > 0)
			break;

	} while (1);

	std::cout << "Fin de la recherche de solutions.\n";
	return 0;
}

// Copyright 2015 Guillaume Lethuillier
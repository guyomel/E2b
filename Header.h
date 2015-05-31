const int  
  PIECES = 256
, TEST = 0
, VERBOSE = 1
, COMPTEUR = 350000
, POPULATION = 10
, MUTATION = 1
;

#define BEEP std::cout << "\007" << std::endl;

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <random>
#include <functional>
#include <iomanip>
#include <unordered_map>

#if defined(WIN32)
#define OS_WIN
#endif

std::random_device rd;
std::mt19937 gen(rd());


/*
//  I/O
// -----
*/


bool fichier_ouverture
(
  std::string Chemin
, std::vector<int>& R_
, std::vector<int>& A_
, std::vector<int>& B_
, std::vector<int>& C_
, std::vector<int>& D_
)
{
	std::string Puzzle = Chemin + "puzzle" + std::to_string(PIECES) + ".txt";
	if (TEST == 1 && PIECES >= 256) Puzzle = Chemin + "TEST_" + std::to_string(PIECES) + ".txt";

	std::ifstream fichier(Puzzle);
	std::string ligne;
	std::string e1, e2, e3, e4, e5;

	if (fichier.good())
	{
		do
		{
			fichier >> e1 >> e2 >> e3 >> e4 >> e5;
			e1.erase(remove(e1.begin(), e1.end(), '.'), e1.end());

			R_.push_back(atoi(e1.c_str()));
			A_.push_back(atoi(e2.c_str()));
			B_.push_back(atoi(e3.c_str()));
			C_.push_back(atoi(e4.c_str()));
			D_.push_back(atoi(e5.c_str()));

		} while (getline(fichier, ligne));
		std::cout << "Ouverture fichier PUZZLE " << PIECES << "\n";
		return true;
	}
	else
		return false;

	fichier.close();
}


bool fichier_hash
(
  std::vector<int>& R_
, std::vector<int>& A_
, std::vector<int>& B_
, std::vector<int>& C_
, std::vector<int>& D_
)
{
	std::string motifs;

	for (int i = 0; i < PIECES; ++i)
	{
		motifs += std::to_string(R_[i]);
		motifs += std::to_string(A_[i]);
		motifs += std::to_string(B_[i]);
		motifs += std::to_string(C_[i]);
		motifs += std::to_string(D_[i]);
	}

	std::hash<std::string> hash;
	std::size_t puzzle_hash = hash(motifs);

	if (puzzle_hash != 1307592716)
	{
		std::cout << "!ERR: echec integrite du fichier (HASH: " << puzzle_hash << ").\n";
		return false;
	}
	else
		return true;
}


void affichage_progression
(
  int& niveau
, int& niveau_ref
, std::vector<int>& progression
, std::vector<int>& R
)
{
	niveau_ref = niveau;

	if (VERBOSE == 1)
	{
		int j;
		std::cout << "\nDescente max : " << niveau + 1 << "/" << PIECES << "\n ";
		std::cout << " Progression  :\n" << " ";

		for (j = 0; j < (int)progression.size(); ++j)
		{
			if (j < 8)
				std::cout << R[progression[j]] << " ";

			else if (j == 8)
				std::cout << ". . . ";

			else if (j > PIECES - 8)
				std::cout << R[progression[j]] << " ";

		} std::cout << "\n ";
	}
}


void enregistrement_solution
(
  std::string& Chemin
, std::vector<int>& R
, std::vector<int>& A
, std::vector<int>& B
, std::vector<int>& C
, std::vector<int>& D
, std::vector<int>& progression
, std::vector<int>& rotations
, int& nb_solutions
)
{
	std::string TXT_solution = Chemin + "Solutions/" + std::to_string(PIECES) + ".txt";
	if (TEST == 1 && PIECES == 256) TXT_solution = Chemin + "Solutions/TEST_" + std::to_string(PIECES) + ".txt";
	std::ofstream solution(TXT_solution, std::ios_base::app | std::ios_base::out);

	solution << "\n & -- SOLUTION --------------------------  \n";
	solution << " > ";
	if (VERBOSE == 1)
	{
		std::cout << "\n  ! SOLUTION\n";
		for (int i = 0; i < PIECES; ++i) std::cout << rotations[i];
		std::cout << "\n " << "\n ";
	}
	for (int i = 0; i < PIECES; ++i) solution << rotations[i];
	solution << "\n " << "\n ";

	for (int i = 0; i < PIECES; ++i)
	{

		if ((i + 1) % (int)sqrt(PIECES) != 0)
			solution << "  ";
		else
		{
			if (i == (PIECES / 2) - 1)
				solution << "__";
			else
				solution << " _";
		}

		solution << i + 1;
		solution << "| " << std::setw(6);
		solution << R[progression[i]] << ".  ";
		solution << A[progression[i]] << " ";
		solution << B[progression[i]] << " ";
		solution << C[progression[i]] << " ";
		solution << D[progression[i]] << "\n";

		if (VERBOSE == 1)
		{
			std::cout << " " << i + 1;
			std::cout << "| " << std::setw(6);
			std::cout << R[progression[i]] << ".  ";
			std::cout << A[progression[i]] << " ";
			std::cout << B[progression[i]] << " ";
			std::cout << C[progression[i]] << " ";
			std::cout << D[progression[i]] << "\n ";
		}
	}

	if (VERBOSE != 1) std::cout << " ( S ) ";

	solution << "\n  ----------------------------------------  \n" << "\n ";

	solution.close();
	nb_solutions++;

	BEEP
}


/*
//  ROTATIONS
// -----------
*/


class Individu
{
public:
	std::vector<int> ADN;
	double fitness;
	bool solution;
};


void rotation_pieces
(
  std::vector<int>& A_
, std::vector<int>& B_
, std::vector<int>& C_
, std::vector<int>& D_
, std::vector<int>& A
, std::vector<int>& B
, std::vector<int>& C
, std::vector<int>& D
, std::vector<int>& ADN
)
{
	int temp[4];

	for (int i = 0; i < PIECES; i++)
	{
		temp[0] = A_[i];
		temp[1] = B_[i];
		temp[2] = C_[i];
		temp[3] = D_[i];

		A[i] = temp[(0 + ADN[i]) % 4];
		B[i] = temp[(1 + ADN[i]) % 4];
		C[i] = temp[(2 + ADN[i]) % 4];
		D[i] = temp[(3 + ADN[i]) % 4];
	}
}


void evaluation_individu
(
  std::vector<int>& A
, std::vector<int>& B
, std::vector<int>& C
, std::vector<int>& D
, double& fitness
)
{
	fitness = 0.0;

	// Zéros
	int zeros[4] = {};

	for (int i = 0; i < PIECES; ++i)
	{
		if (A[i] == 0)
			zeros[0]++;

		if (B[i] == 0)
			zeros[1]++;

		if (C[i] == 0)
			zeros[2]++;

		if (D[i] == 0)
			zeros[3]++;
	}

	if (zeros[0] != zeros[1]
		|| zeros[1] != zeros[2]
		|| zeros[2] != zeros[3])
		fitness += 100;

	// Bijections
	std::vector<int> A_eval, B_eval, C_eval, D_eval;
	std::vector<int> intersection;

	A_eval = A; B_eval = B; C_eval = C; D_eval = D;

	std::sort(A_eval.begin(), A_eval.end());
	std::sort(C_eval.begin(), C_eval.end());

	set_intersection(A_eval.begin(), A_eval.end(),
		C_eval.begin(), C_eval.end(),
		back_inserter(intersection));

	fitness += PIECES - intersection.size();

	intersection.clear();

	std::sort(B_eval.begin(), B_eval.end());
	std::sort(D_eval.begin(), D_eval.end());

	set_intersection(B_eval.begin(), B_eval.end(),
		D_eval.begin(), D_eval.end(),
		back_inserter(intersection));

	fitness += PIECES - intersection.size();

	fitness = 1 / (fitness + 1);
}


void reproduction
(
  std::vector<int>& ADN_A
, std::vector<int>& ADN_B
)
{
	for (int i = 0; i < PIECES; ++i)
	{
		if (i >(50 * PIECES) / 100)
			ADN_A[i] = ADN_B[i];
	}
}


/*
//  EMPLACEMENTS
// --------------
*/


class Noeud
{
public:
	std::vector<int> branche;
};


void liste_noeuds
(
  std::vector<int>& progression
, std::vector<int>& branche
, std::vector<int>& restants
, std::vector<int>& A
, std::vector<int>& B
, std::vector<int>& C
, std::vector<int>& D
, int& nb_lignes
, int& niveau
)
{
	int j;
	for (j = 0; j < (int)restants.size(); ++j)
	{
		if (
			B[progression[niveau]] == D[restants[j]]
			&& (niveau <= nb_lignes
			|| niveau > PIECES - nb_lignes)
			)
		{
			branche.push_back(restants[j]);
		}
		else if (
			B[progression[niveau]] == D[restants[j]]
			&& A[restants[j]] == C[progression[niveau + 1 - nb_lignes]]
			)
		{
			branche.push_back(restants[j]);
		}
	}

}


bool oracle
(
  int& actif
, std::vector<int>& restants
, std::vector<int>& B
, std::vector<int>& D
)
{
	std::vector<int> B_oracle, D_oracle, intersection;

	for (int i = 0; i < (int)restants.size(); ++i)
	{
		B_oracle.push_back(B[restants[i]]);
		D_oracle.push_back(D[restants[i]]);
	}

	// Supprimer D[actif] *une seule fois*
	auto it = find(D_oracle.begin(), D_oracle.end(), D[actif]);
	if (it != D_oracle.end())
		D_oracle.erase(it);

	D_oracle.shrink_to_fit();
	D_oracle.push_back(0);

	sort(B_oracle.begin(), B_oracle.end());
	sort(D_oracle.begin(), D_oracle.end());

	set_intersection(B_oracle.begin(), B_oracle.end(),
		D_oracle.begin(), D_oracle.end(),
		back_inserter(intersection));

	if (restants.size() - intersection.size() == 0)
		return true;
	else
		return false;
}


void filtrage_noeuds
(
  std::vector<int>& branche
, std::vector<int>& restants
, std::vector<int>& A
, std::vector<int>& B
, std::vector<int>& C
, std::vector<int>& D
, int& niveau
)
{
	int j;
	int nb_lignes = (int)sqrt(PIECES);

	if (niveau == 0)
	{
		for (j = 0; j < (int)branche.size(); ++j)
			if (A[branche[j]] != 0)
			{
				branche.erase(branche.begin() + j);
				branche.shrink_to_fit();
			}

		if ((int)branche.size() > 0)
			for (j = 0; j < (int)branche.size(); ++j)
				if (!(oracle(branche[j], restants, B, D)))
				{
					branche.erase(branche.begin() + j);
					branche.shrink_to_fit();
				}
	}
	else
	{
		// > bordure haut
		if (niveau + 1 < nb_lignes)
			for (j = branche.size() - 1; j >= 0; --j)
				if (A[branche[j]] != 0)
					branche.erase(branche.begin() + j);

		// Bordure droite
		if ((niveau + 1) % nb_lignes + 1 == 0)
			for (j = branche.size() - 1; j >= 0; --j)
				if (B[branche[j]] != 0)
					branche.erase(branche.begin() + j);

		// > bordure gauche
		if ((niveau + 1) % nb_lignes == 0)
			for (j = branche.size() - 1; j >= 0; --j)
				if (D[branche[j]] != 0)
					branche.erase(branche.begin() + j);

		// > coins
		if (niveau + 1 == nb_lignes - 1)
			for (j = branche.size() - 1; j >= 0; --j)
				if (B[branche[j]] != 0)
					branche.erase(branche.begin() + j);

		if (niveau + 1 == PIECES - nb_lignes)
			for (j = branche.size() - 1; j >= 0; --j)
				if (C[branche[j]] != 0
					|| D[branche[j]] != 0)
					branche.erase(branche.begin() + j);

		// > internes
		if (niveau + 1 > nb_lignes && niveau + 1 < PIECES - nb_lignes)
			for (j = branche.size() - 1; j >= 0; --j)
				if (C[branche[j]] == 0)
					branche.erase(branche.begin() + j);

		// > oracle
		if (branche.size() > 0)
			for (j = 0; j < (int)branche.size(); ++j)
				if (!(oracle(branche[j], restants, B, D)))
					branche.erase(branche.begin() + j);

		branche.shrink_to_fit();
	}
}


void controle_solution
(
  std::vector<int>& progression
, std::vector<int>& R
, std::vector<int>& A
, std::vector<int>& B
, std::vector<int>& C
, std::vector<int>& D
, std::string& Chemin
, int& nb_solutions
, std::vector<int>& rotations
)
{
	int decalage = (int)sqrt(PIECES);
	bool bijection = true;

	std::vector<int> A_controle, C_controle;

	for (int i = 0; i < PIECES; ++i)
	{
		A_controle.push_back(A[progression[i]]);
		C_controle.push_back(C[progression[i]]);
	}

	for (int i = 0; i < PIECES; ++i)
	{
		if (C_controle[i] != A_controle[(i + decalage) % PIECES])
		{
			bijection = false;
			break;
		}
	}

	if (bijection)
		enregistrement_solution(Chemin, R, A, B, C, D, progression, rotations, nb_solutions);
}
/*
 *  lacgfp8.c
 *  StochMod
 *
 *  This file is part of libStochMod.
 *  Copyright 2011-2017 Gabriele Lillacci.
 *
 *  libStochMod is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libStochMod is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libStochMod.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../stochmod.h"


// Number of species
#define N 8
// Number of reactions
#define R 15
// Number of parameters
#define L 15
// Number of inputs
#define Z 1
// Number of outputs
#define P 1


/*
 === SPECIES ===
	X1	--->	lacI		lacI mRNA
	X2	--->	LACI		LACI protein monomer
	X3	--->	LACI2		LACI dimer
	X4	--->	PLac		Unoccopied (active) Lac promoter
	X5	--->	O2Lac		Occupied promoter bound to LACI dimer
	X6	--->	gfp			gfp mRNA
	X7	--->	GFP			GFP protein (dark)
	X8	--->	mGFP		GFP protein (mature)

 === REACTIONS ===
	reaction 1.		NULL --(k1)--> lacI				Transcription of lacI mRNA (constitutive)
	reaction 2.		lacI --(k2)--> NULL				Degradation of lacI mRNA (constitutive)
	reaction 3.		lacI --(k3)--> lacI + LACI		Translation of LACI protein
	reaction 4.		LACI --(k4+k5*u1)--> NULL		Degradation of LACI protein, increased by the input (IPTG)
	reaction 5.		LACI + LACI --(k6)--> LACI2		Dimerization of LACI protein
	reaction 6.		LACI2 --(k7)--> LACI + LACI		Dissociation of LACI dimer
	reaction 7.		LACI2 + PLac --(k8)--> O2Lac	Binding of LACI dimer to Lac operator sequence
	reaction 8.		O2Lac --(k9)--> LACI2 + PLac	Dissociation of LACI dimer from operator sequence
	reaction 9.		PLac --(k10)--> PLac + gfp		Transcription of gfp mRNA from active Lac promoter
	reaction 10.	O2Lac --(k11)--> O2Lac + gfp	Transcription of gfp mRNA from Lac promoter bound to LacI dimer
	reaction 11.	gfp --(k12)--> NULL				Degradation of gfp mRNA
	reaction 12.	gfp --(k13)--> gfp + GFP		Translation of dark GFP protein
	reaction 13.	GFP --(k14)--> NULL				Degradation of dark GFP protein
	reaction 14.	GFP --(k15)--> mGFP				Maturation of GFP
	reaction 15.	mGFP --(k14)--> NULL			Degradation of mature GFP protein
*/

/**
 Propensity evaluation function for Lacgfp8.
 */
int lacgfp8_propensity_eval (const gsl_vector * X, const gsl_vector * params, gsl_vector * prop)
{
	// Check sizes of vectors
	if ((X->size != N) || (params->size != L+Z) || (prop->size != R))
	{
		fprintf (stderr, "error in lacgfp8_propensity_eval: vector sizes are not correct\n");
		fprintf (stderr, "\tstate: %d - params: %d - propensities: %d\n", (int) X->size, (int) params->size, (int) prop->size);
		return GSL_EFAILED;
	}

	// Recover species from X vector
	double X1 = gsl_vector_get (X, 0);
	double X2 = gsl_vector_get (X, 1);
	double X3 = gsl_vector_get (X, 2);
	double X4 = gsl_vector_get (X, 3);
	double X5 = gsl_vector_get (X, 4);
	double X6 = gsl_vector_get (X, 5);
	double X7 = gsl_vector_get (X, 6);
	double X8 = gsl_vector_get (X, 7);

	// Recover parameters from params vector
	double k1 = gsl_vector_get (params, 0);
	double k2 = gsl_vector_get (params, 1);
	double k3 = gsl_vector_get (params, 2);
	double k4 = gsl_vector_get (params, 3);
	double k5 = gsl_vector_get (params, 4);
	double k6 = gsl_vector_get (params, 5);
	double k7 = gsl_vector_get (params, 6);
	double k8 = gsl_vector_get (params, 7);
	double k9 = gsl_vector_get (params, 8);
	double k10 = gsl_vector_get (params, 9);
	double k11 = gsl_vector_get (params, 10);
	double k12 = gsl_vector_get (params, 11);
	double k13 = gsl_vector_get (params, 12);
	double k14 = gsl_vector_get (params, 13);
	double k15 = gsl_vector_get (params, 14);

	// Recover the input
	double u1 = gsl_vector_get (params, 15);


	// Evaluate the propensities
	gsl_vector_set (prop, 0, (k1));
	gsl_vector_set (prop, 1, (k2)*X1);
	gsl_vector_set (prop, 2, (k3)*X1);
	gsl_vector_set (prop, 3, (k4+k5*u1)*X2);
	gsl_vector_set (prop, 4, (k6)*X2*(X2-1));
	gsl_vector_set (prop, 5, (k7)*X3);
	gsl_vector_set (prop, 6, (k8)*X3*X4);
	gsl_vector_set (prop, 7, (k9)*X5);
	gsl_vector_set (prop, 8, (k10)*X4);
	gsl_vector_set (prop, 9, (k11)*X5);
	gsl_vector_set (prop, 10, (k12)*X6);
	gsl_vector_set (prop, 11, (k13)*X6);
	gsl_vector_set (prop, 12, (k14)*X7);
	gsl_vector_set (prop, 13, (k15)*X7);
	gsl_vector_set (prop, 14, (k14)*X8);

	// Signal that computation was completed successfully
	return GSL_SUCCESS;
}


/**
 State update function for Lacgfp8.
 */
int lacgfp8_state_update (gsl_vector * X, size_t rxnid)
{
	// Check sizes of state vector
	if (X->size != N)
	{
		fprintf (stderr, "error in lacgfp8_state_update: state vector size is not correct\n");
		return GSL_EFAILED;
	}

	// Check that reaction id is correct
	if (rxnid >= R)
	{
		fprintf (stderr, "error in lacgfp8_state_update: reaction id is not correct\n");
		return GSL_EFAILED;
	}

	// Update the state vector according to which reaction fired
	switch (rxnid)
	{
	case 0:
		gsl_vector_set (X, 0, gsl_vector_get (X, 0) + (1));
		break;

	case 1:
		gsl_vector_set (X, 0, gsl_vector_get (X, 0) + (-1));
		break;

	case 2:
		gsl_vector_set (X, 1, gsl_vector_get (X, 1) + (1));
		break;

	case 3:
		gsl_vector_set (X, 1, gsl_vector_get (X, 1) + (-1));
		break;

	case 4:
		gsl_vector_set (X, 1, gsl_vector_get (X, 1) + (-2));
		gsl_vector_set (X, 2, gsl_vector_get (X, 2) + (1));
		break;

	case 5:
		gsl_vector_set (X, 1, gsl_vector_get (X, 1) + (2));
		gsl_vector_set (X, 2, gsl_vector_get (X, 2) + (-1));
		break;

	case 6:
		gsl_vector_set (X, 2, gsl_vector_get (X, 2) + (-1));
		gsl_vector_set (X, 3, gsl_vector_get (X, 3) + (-1));
		gsl_vector_set (X, 4, gsl_vector_get (X, 4) + (1));
		break;

	case 7:
		gsl_vector_set (X, 2, gsl_vector_get (X, 2) + (1));
		gsl_vector_set (X, 3, gsl_vector_get (X, 3) + (1));
		gsl_vector_set (X, 4, gsl_vector_get (X, 4) + (-1));
		break;

	case 8:
		gsl_vector_set (X, 5, gsl_vector_get (X, 5) + (1));
		break;

	case 9:
		gsl_vector_set (X, 5, gsl_vector_get (X, 5) + (1));
		break;

	case 10:
		gsl_vector_set (X, 5, gsl_vector_get (X, 5) + (-1));
		break;

	case 11:
		gsl_vector_set (X, 6, gsl_vector_get (X, 6) + (1));
		break;

	case 12:
		gsl_vector_set (X, 6, gsl_vector_get (X, 6) + (-1));
		break;

	case 13:
		gsl_vector_set (X, 6, gsl_vector_get (X, 6) + (-1));
		gsl_vector_set (X, 7, gsl_vector_get (X, 7) + (1));
		break;

	case 14:
		gsl_vector_set (X, 7, gsl_vector_get (X, 7) + (-1));
		break;
	}

	// Signal that computation was completed correctly
	return GSL_SUCCESS;
}


/**
 Sample a new random initial state for Lacgfp8.
 */
int lacgfp8_initial_conditions (gsl_vector * X0, const gsl_rng * r)
{
	// Check sizes of state vector
	if (X0->size != N)
	{
		fprintf (stderr, "error in lacgfp8_initial_conditions: state vector size is not correct\n");
		return GSL_EFAILED;
	}

	// Sample a new initial state
	gsl_vector_set (X0, 0, gsl_rng_uniform_int (r, 6));
	gsl_vector_set (X0, 1, gsl_rng_uniform_int (r, 11));
	gsl_vector_set (X0, 2, 0);
	gsl_vector_set (X0, 3, 1 + gsl_rng_uniform_int (r, 101) + gsl_rng_uniform_int (r, 101));
	// gsl_vector_set (X0, 3, 50 + gsl_rng_uniform_int (r, 21));
	gsl_vector_set (X0, 4, 0);
	gsl_vector_set (X0, 5, 0);
	gsl_vector_set (X0, 6, 0);
	gsl_vector_set (X0, 7, 0);

	// Signal that computation was completed correctly
	return GSL_SUCCESS;
}


/**
 Output function for Lacgfp8.
 */
int lacgfp8_output (gsl_matrix * out)
{
	if ((out->size1 != P) || (out->size2 != N))
	{
		fprintf (stderr, "error in lacgfp8_output: output matrix size is not correct\n");
		return GSL_EFAILED;
	}

	// Reset the output matrix
	gsl_matrix_set_all (out, 0.0);

	// Set the non-zero terms
	gsl_matrix_set (out, 0, 7, 1.0);

	// Signal that computation was completed correctly
	return GSL_SUCCESS;
}


/**
 Model information function for Lacgfp8.
 */
void lacgfp8_mod_setup (stochmod * model)
{
	model->propensity = &lacgfp8_propensity_eval;
	model->update = &lacgfp8_state_update;
	model->initial = &lacgfp8_initial_conditions;
	model->output = &lacgfp8_output;
	model->nspecies = N;
	model->nrxns = R;
	model->nparams = L;
	model->nin = Z;
	model->nout = P;
	model->name = "Lac-GFP construct model v8 (LACGFP8)";
}

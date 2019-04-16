#include <criterion/criterion.h>

#include "float.h"
#include "math.h"
#include "utils.h"
#include "objectives.h"
#include "penguin.h"

Test(macro_tests, min_and_max) {
  cr_assert(0 == pen_min(0, 10), "minimum macro 1");
  cr_assert(-10 == pen_min(0.0, -10), "minimum macro 2");
  cr_assert(10 == pen_max(0, 10), "maximum macro 1");
  cr_assert(10 == pen_max(-INFINITY, 10), "maximum macro 2");
}


Test(penguin_unit, random_min_max) {
  srand((unsigned) time(NULL));

  double r = random_min_max(0.0, 1.0);
  cr_assert(r <= 1.0, "random_min_max upper bound 1");
  cr_assert(r >= 0.0, "random_min_max lower bound 1");

  r = random_min_max(-5.0, 0.0);
  cr_assert(r <= 0.0, "random_min_max upper bound 2");
  cr_assert(r >= -5.0, "random_min_max lower bound 2");

  r = random_min_max(-100.0, 100.0);
  cr_assert(r <= 100.0, "random_min_max upper bound 3");
  cr_assert(r >= -100.0, "random_min_max lower bound 3");

  r = random_min_max(0.0, 0.0);
  cr_assert(r <= 0.0, "random_min_max upper bound 4");
  cr_assert(r >= 0.0, "random_min_max lower bound 4");
}


Test(penguin_unit, generate_population) {
  srand((unsigned) time(NULL));

  // Generating the max and min values allowed for every dimension.
  double max[] = {100.0, 0.0, 11.0, -2.0};
  double min[] = {-100.0, -5.0, 10.0, -4.0};

  // Generating population of size 100 with dim 4.
  double *population = pen_generate_population(100, 4, min, max);

  // Checking if the initial values of the penguin population are within the specified bounds.
  for (size_t idx = 0; idx < 4 * 100; idx += 4) {
    cr_assert(population[idx + 0] <= 100.0, "first dimension upper bound check");
    cr_assert(population[idx + 0] >= -100.0, "first dimension lower bound check");
    cr_assert(population[idx + 1] <= 0.0, "second dimension upper bound check");
    cr_assert(population[idx + 1] >= -5.0, "second dimension lower bound check");
    cr_assert(population[idx + 2] <= 11.0, "third dimension upper bound check");
    cr_assert(population[idx + 2] >= 10.0, "third dimension lower bound check");
    cr_assert(population[idx + 3] <= -2.0, "fourth dimension upper bound check");
    cr_assert(population[idx + 3] >= -4.0, "fourth dimension lower bound check");
  }
  free(population);
}


Test(penguin_unit, copy_population) {
  srand((unsigned) time(NULL));

  size_t population_size = 100;
  size_t population_dim = 4;
  double max[] = {100.0, 0.0, 11.0, -2.0};
  double min[] = {-100.0, -5.0, 10.0, -4.0};
  double *population = pen_generate_population(population_size, population_dim, min, max);
  double *population_copy = pen_copy_population(population_size, population_dim, population);

  // Check that values in original population and copy are the same.
  // Note the documentation of criterion is false. The third parameter needs to be the size.
  cr_assert_arr_eq(population, population_copy, population_size);

  // Check that original and copy are at different locations in memory.
  cr_assert_neq(population, population_copy);

  free(population);
  free(population_copy);
}


Test(penguin_unit, get_initial_fitness) {
  double population[] = {1.0, 2.0, 3.0, 4.0, 5.0, -5.0, 10, -1.0};

  // Getting the initial fitness of population of size 4 and dim 2 by using the summation function.
  double *fitness = pen_get_initial_fitness(4, 2, population, sum);

  cr_assert_eq(fitness[0], 3.0, "Fitness of penguin 0 incorrect.");
  cr_assert_eq(fitness[1], 7.0, "Fitness of penguin 1 incorrect.");
  cr_assert_eq(fitness[2], 0.0, "Fitness of penguin 2 incorrect.");
  cr_assert_eq(fitness[3], 9.0, "Fitness of penguin 3 incorrect.");

  free(fitness);
}


Test(penguin_unit, eucledian_distance) {
  double point1[] = {10.0, 10.0};
  double point2[] = {10.0, 11.0};
  double dist = pen_eucledian_distance(2, point1, point2);
  cr_assert_eq(dist, 1.0, "Distance test 1 failed.");

  point1[0] = 0.0;
  point1[1] = 0.0;
  point2[0] = 3.0;
  point2[1] = 4.0;
  dist = pen_eucledian_distance(2, point1, point2);
  cr_assert_eq(dist, 5.0, "Distance test 2 failed.");
}


Test(penguin_unit, clamp_position) {
  double position[] = {-105, -90,   100,  1000, 0  };
  double max[]      = { 100,  100,  100,  100,  100};
  double min[]      = {-100, -100, -100, -100, -100};
  pen_clamp_position(5, position, min, max);

  cr_assert_eq(position[0], -100, "1st dimension clamp failed.");
  cr_assert_eq(position[1], -90,  "2nd dimension clamp failed.");
  cr_assert_eq(position[2], 100,  "3rd dimension clamp failed.");
  cr_assert_eq(position[3], 100,  "4th dimension clamp failed.");
  cr_assert_eq(position[4], 0,    "5th dimension clamp failed.");
}


Test(penguin_unit, mutate) {
  double original[] = {0.0, 0.0, 0.0, 0.0};
  size_t dim = 4;

  // Test whether permutation happens even though mutation_coef = 0.
  double mutation_coef = 0.0;
  double expected[] = {0.0, 0.0, 0.0, 0.0};
  pen_mutate(dim, original, mutation_coef);
  cr_assert_arr_eq(original, expected, dim, "For mutation coef zero, no mutation should happen.");

  // Test whether permuted values stay in range of expected values.
  mutation_coef = 1.0;
  pen_mutate(dim, original, 1.0);
  for (size_t idx = 0; idx < 4; idx++) {
    cr_assert_gt(original[idx], -1.0 * mutation_coef, "Permuted value should be larger than lower bound.");
    cr_assert_lt(original[idx], 1.0 * mutation_coef, "Permuted value should be smaller than upper bound.");
  }

  // Same test as before for more complex scenario.
  original[0] = 10.0;
  original[1] = -10.0;
  original[2] = 0.0;
  original[3] = 0.5;
  mutation_coef = 0.5;
  pen_mutate(dim, original, mutation_coef);

  cr_assert_lt(fabs(original[0] - 10.0), 0.5, "First dimension out of bounds.");
  cr_assert_lt(fabs(original[1] + 10.0), 0.5, "Second dimension out of bounds.");
  cr_assert_lt(fabs(original[2] - 0.0), 0.5, "Third dimension out of bounds.");
  cr_assert_lt(fabs(original[3] - 0.5), 0.5, "Fourth dimension out of bounds.");
}


Test(penguin_unit, attractiveness) {
  size_t dim = 4;
  double penguin_i[] = {0.0, 0.0, 0.0, 0.0};
  double penguin_j[] = {0.0, 4.0, 3.0, 6.0};
  double penguin_k[] = {1.0, 1.0, 1.0, 0.0};
  double fitness[] = {100.0, 10.0, 9.0};

  double heat_rad_i = pen_heat_radiation(fitness[0]);
  double heat_rad_j = pen_heat_radiation(fitness[1]);
  double heat_rad_k = pen_heat_radiation(fitness[2]);

  cr_assert(heat_rad_i > heat_rad_j, "Penguin i should radiate more than penguin j.");
  cr_assert(heat_rad_i > heat_rad_k, "Penguin i should radiate more than penguin k.");
  cr_assert(heat_rad_j > heat_rad_k, "Penguin j should radiate more than penguin k.");

  double attractiveness_i_j = pen_attractiveness(heat_rad_i, dim, penguin_i, penguin_j, ATT_COEF_START);
  double attractiveness_j_i = pen_attractiveness(heat_rad_j, dim, penguin_i, penguin_j, ATT_COEF_START);
  cr_assert(attractiveness_i_j > attractiveness_j_i, "penguin j should be more attractive to "
                                                     "penguin i than the other way around");
  double attractiveness_i_j_bis = pen_attractiveness(heat_rad_i, 4, penguin_j, penguin_i, ATT_COEF_START);
  cr_assert(fabs(attractiveness_i_j - attractiveness_i_j_bis) < DBL_EPSILON, "order of penguins in "
                                                                             "arguments should not matter");

  double attractiveness_i_k = pen_attractiveness(heat_rad_i, 4, penguin_i, penguin_k, ATT_COEF_START);
  cr_assert(attractiveness_i_j < attractiveness_i_k, "penguin k should be more attractive to "
                                                     "penguin i than penguin j is attracted to penguin i, since k is closer to i");

  double attractiveness_j_k = pen_attractiveness(heat_rad_j, 4, penguin_j, penguin_k, ATT_COEF_START);
  cr_assert(attractiveness_i_j > attractiveness_j_k, "penguin j should be more attractive to "
                                                     "penguin i than penguin k is attracted to penguin j, since i radiates much more "
                                                     "energy");
}
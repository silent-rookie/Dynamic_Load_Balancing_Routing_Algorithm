def generate_description_with_GEO(
        filename_description,
        max_gsl_length_m,
        max_isl_length_m,
        max_ill_length_m
):
    with open(filename_description, "w+") as f_out:
        f_out.write("max_gsl_length_m=%.10f\n" % max_gsl_length_m)
        f_out.write("max_isl_length_m=%.10f\n" % max_isl_length_m)
        f_out.write("max_ill_length_m=%.10f\n" % max_ill_length_m)
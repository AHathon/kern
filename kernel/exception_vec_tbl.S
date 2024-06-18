.global arm64_excep_vec_tbl;
.type arm64_excep_vec_tbl, %function;

arm64_excep_vec_tbl:
	b undef_vec_ent
	b undef_vec_ent
	b undef_vec_ent
	b undef_vec_ent
	
	b current_el1_sync
	b current_el1_irq
	b undef_vec_ent
	b current_el1_serr
	
	b lower_el1_sync
	b lower_el1_irq
	b undef_vec_ent
	b lower_el1_serr
	
	b undef_vec_ent
	b undef_vec_ent
	b undef_vec_ent
	b undef_vec_ent
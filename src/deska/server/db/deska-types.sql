SET search_path TO deska;

CREATE DOMAIN identifier AS text
	CONSTRAINT digit_on_first_place
		CHECK ( VALUE !~ '^[[:digit:]]' )
	CONSTRAINT not_alfanumeric_in_name
		CHECK ( VALUE !~ '[^[:alnum:]]' )

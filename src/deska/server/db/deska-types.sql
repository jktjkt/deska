SET search_path TO deska;

CREATE DOMAIN identifier AS text
	CONSTRAINT digit_on_first_place
		CHECK ( VALUE !~ '^[[:digit:]]' )
	CONSTRAINT not_alfanumeric_in_name
		CHECK ( VALUE !~ '[^[:alnum:]]' );

CREATE DOMAIN ipv6 AS inet
	CONSTRAINT is_ipv6
		CHECK ( family(VALUE) = 6 );

CREATE DOMAIN ipv4 AS inet
	CONSTRAINT is_ipv4
		CHECK ( family(VALUE) = 4 );
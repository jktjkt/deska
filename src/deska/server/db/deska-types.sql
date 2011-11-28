SET search_path TO deska;

CREATE DOMAIN identifier AS text
	CONSTRAINT digit_on_first_place
		CHECK ( VALUE !~ '^[[:digit:]]' )
	CONSTRAINT not_alfanumeric_in_name
		CHECK ( VALUE !~ '[^[:alnum:]_-]' )
	CONSTRAINT not_begins_with_hyphen
		CHECK ( VALUE !~ '^-' )
	CONSTRAINT not_ends_with_hyphen
		CHECK ( VALUE !~ '-$' )
	CONSTRAINT length_gt_zero
		CHECK ( char_length(VALUE) > 0 );

CREATE DOMAIN ipv6 AS inet
	CONSTRAINT is_ipv6
		CHECK ( family(VALUE) = 6 );

CREATE DOMAIN ipv4 AS inet
	CONSTRAINT is_ipv4
		CHECK ( family(VALUE) = 4 );

CREATE DOMAIN identifier_set AS bigint;

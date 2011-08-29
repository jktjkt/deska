SET search_path TO deska;

CREATE FUNCTION ret_id_set(id_set text[])
RETURNS text[]
AS
$$
DECLARE
    len int;
BEGIN
    CASE
        WHEN array_upper(id_set,1) = 0 OR array_upper(id_set,1) IS NULL THEN
        RETURN NULL;
        WHEN array_upper(id_set,1) = 1 AND id_set[1] IS NULL THEN
        RETURN cast(ARRAY[] as text[]);
        ELSE
        RETURN id_set;
    END CASE;
END;
$$
LANGUAGE plpgsql;

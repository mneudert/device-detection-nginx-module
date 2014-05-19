use Test::Nginx::Socket;

repeat_each(1);
plan tests => repeat_each() * blocks();

run_tests();

__DATA__

=== TEST 1: tests working
--- config
--- request
    GET /
--- response_code: 200

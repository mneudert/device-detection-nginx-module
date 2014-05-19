use Test::Nginx::Socket;

repeat_each(1);
plan tests => repeat_each() * 2;

run_tests();

__DATA__

=== TEST 1: device_detection_type default value
--- config
    location /type {
        content_by_lua "ngx.print(ngx.var['device_detection_type'])";
    }
--- request
    GET /type
--- response_code: 200
--- response_body: undetected

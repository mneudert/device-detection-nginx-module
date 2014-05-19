use File::Spec;
use Test::Nginx::Socket;

# setup testing environment
my $HtmlDir    = html_dir();
my $FixtureDir = File::Spec->catfile($HtmlDir, '..', '..', 'fixtures');

$ENV{TEST_NGINX_FIXTURE_DIR} = $FixtureDir;

# proceed with testing
repeat_each(1);
plan tests => repeat_each() * 2 * blocks();

no_root_location();
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

=== TEST 2: device_detection_loaded without configured source yaml
--- config
    location /loaded_off {
        content_by_lua "ngx.print(ngx.var['device_detection_loaded'])";
    }
--- request
    GET /loaded_off
--- response_code: 200
--- response_body: off

=== TEST 3: device_detection_loaded with configured source yaml
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /loaded_on {
        content_by_lua "ngx.print(ngx.var['device_detection_loaded'])";
    }
--- request
    GET /loaded_on
--- response_code: 200
--- response_body: on

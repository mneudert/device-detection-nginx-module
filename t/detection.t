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

=== TEST 1: undetected device type
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /undetected {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'i am the night')";
        content_by_lua "ngx.print(ngx.var['device_detection_type'])";
    }
--- request
    GET /undetected
--- response_code: 200
--- response_body: undetected

=== TEST 2: default device
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /default {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'Mozilla/5.0 (Linux; U; Android 4.2.1; en-gb; Nexus 10 Build/JOP40D) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Safari/534.30')";
        content_by_lua "ngx.print(ngx.var['device_detection_type'])";
    }
--- request
    GET /default
--- response_code: 200
--- response_body: smartphone

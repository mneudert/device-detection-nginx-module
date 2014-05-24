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

=== TEST 1: undetected model
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /undetected {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'i am the night')";
        content_by_lua "ngx.print(ngx.var['device_detection_model'])";
    }
--- request
    GET /undetected
--- response_code: 200
--- response_body: undetected

=== TEST 2: Glass model
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /glass {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'Mozilla/5.0 (Linux; U; Android 4.0.4; en-us; Glass 1 Build/IMM76L; XE7) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30')";
        content_by_lua "ngx.print(ngx.var['device_detection_model'])";
    }
--- request
    GET /glass
--- response_code: 200
--- response_body: Glass

=== TEST 3: iPhone model
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /iphone {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_1_2 like Mac OS X; de-de) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7D11 Safari/528.16')";
        content_by_lua "ngx.print(ngx.var['device_detection_model'])";
    }
--- request
    GET /iphone
--- response_code: 200
--- response_body: iPhone

=== TEST 4: Apple TV device
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /appletv {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'XBMC/PRE-11.0 Git:20110623-62171b3 (iOS; 11.0.0 AppleTV2,1; http://www.xbmc.org)')";
        content_by_lua "ngx.print(ngx.var['device_detection_model'])";
    }
--- request
    GET /appletv
--- response_code: 200
--- response_body: Apple TV

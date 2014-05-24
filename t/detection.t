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

=== TEST 2: feature phone device
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /feature_phone {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'Mozilla/4.0 (compatible; MSIE 6.0; Windows CE; IEMobile 6.12; es-US; KIN.Two 1.0)')";
        content_by_lua "ngx.print(ngx.var['device_detection_type'])";
    }
--- request
    GET /feature_phone
--- response_code: 200
--- response_body: feature phone

=== TEST 3: smart display device
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /smart_display {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'Mozilla/5.0 (Linux; U; Android 4.0.4; de-de; VSD220 Build/IMM76D.UI23ED12_VSC) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Safari/534.30')";
        content_by_lua "ngx.print(ngx.var['device_detection_type'])";
    }
--- request
    GET /smart_display
--- response_code: 200
--- response_body: smart display

=== TEST 4: smartphone device
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /smartphone {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_1_2 like Mac OS X; de-de) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7D11 Safari/528.16')";
        content_by_lua "ngx.print(ngx.var['device_detection_type'])";
    }
--- request
    GET /smartphone
--- response_code: 200
--- response_body: smartphone

=== TEST 5: tablet device
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /tablet {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'Mozilla/5.0 (Linux; Android 4.4.2; Nexus 10 Build/KOT49H) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/33.0.1750.136 Safari/537.36')";
        content_by_lua "ngx.print(ngx.var['device_detection_type'])";
    }
--- request
    GET /tablet
--- response_code: 200
--- response_body: tablet

=== TEST 6: tv device
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /tv {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'XBMC/PRE-11.0 Git:20110623-62171b3 (iOS; 11.0.0 AppleTV2,1; http://www.xbmc.org)')";
        content_by_lua "ngx.print(ngx.var['device_detection_type'])";
    }
--- request
    GET /tv
--- response_code: 200
--- response_body: tv

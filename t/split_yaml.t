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

=== TEST 1: split yaml (find primary)
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/split_yaml_1.yml;
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/split_yaml_2.yml;

    location /primary {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'detect from primary yaml')";
        content_by_lua "ngx.print(ngx.var['device_detection_type'])";
    }
--- request
    GET /primary
--- response_code: 200
--- response_body: primary device

=== TEST 2: split yaml (find secondary)
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/split_yaml_1.yml;
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/split_yaml_2.yml;

    location /secondary {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'detect from secondary yaml')";
        content_by_lua "ngx.print(ngx.var['device_detection_type'])";
    }
--- request
    GET /secondary
--- response_code: 200
--- response_body: secondary device

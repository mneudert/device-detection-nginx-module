use File::Spec;
use Test::Nginx::Socket;

# setup testing environment
my $HtmlDir    = html_dir();
my $FixtureDir = File::Spec->catfile($HtmlDir, '..', '..', 'fixtures');
my $LogDir     = File::Spec->catfile($HtmlDir, '..', 'logs');

$ENV{TEST_NGINX_FIXTURE_DIR} = $FixtureDir;
$ENV{TEST_NGINX_LOG_DIR}     = $LogDir;

# proceed with testing
repeat_each(1);
plan tests => repeat_each() * 2 * blocks();

no_root_location();
run_tests();

__DATA__

=== TEST 1: log undetected type
--- config
    device_detection_miss_log  $TEST_NGINX_LOG_DIR/miss.log;
    device_detection_yaml      $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /type {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'i am the night')";

        content_by_lua "
            local type = ngx.var['device_detection_type']
            local log  = '$TEST_NGINX_LOG_DIR/miss.log'

            for line in io.lines(log) do
                ngx.print(line)
            end
        ";
    }
--- request
    GET /type
--- response_code: 200
--- response_body: i am the night

=== TEST 1: log undetected model
--- config
    device_detection_miss_log  $TEST_NGINX_LOG_DIR/miss.log;
    device_detection_yaml      $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /model {
        rewrite_by_lua "ngx.req.set_header('User-Agent', 'i am the night')";

        content_by_lua "
            local type = ngx.var['device_detection_model']
            local log  = '$TEST_NGINX_LOG_DIR/miss.log'

            for line in io.lines(log) do
                ngx.print(line)
            end
        ";
    }
--- request
    GET /model
--- response_code: 200
--- response_body: i am the night

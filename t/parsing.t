use File::Spec;
use Test::Nginx::Socket;

# setup testing environment
my $HtmlDir    = html_dir();
my $FixtureDir = File::Spec->catfile($HtmlDir, '..', '..', 'fixtures');

$ENV{TEST_NGINX_FIXTURE_DIR} = $FixtureDir;

# proceed with testing
repeat_each(1);
plan tests => repeat_each();

no_root_location();
run_tests();

__DATA__

=== TEST 1: load regexes form fixture directory
--- config
    device_detection_yaml  $TEST_NGINX_FIXTURE_DIR/devices.yml;

    location /loaded {
        content_by_lua "ngx.print('file loaded!')";
    }
--- request
    GET /loaded
--- response_code: 200

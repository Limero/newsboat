#include "download.h"

#include "test-helpers.h"

using namespace podboat;

TEST_CASE("Require-view-update callback gets called when download progress or status changes",
	"[Download]")
{
	GIVEN("A Download object") {
		bool got_called = false;
		std::function<void()> callback = [&got_called]() {
			got_called = true;
		};

		Download d(callback);

		REQUIRE(d.status() == DlStatus::QUEUED);

		WHEN("its progress is updated (increased)") {
			d.set_progress(1.0, 1.0);

			THEN("the require-view-update callback gets called") {
				REQUIRE(got_called);
			}
		}

		WHEN("its progress has not changed") {
			d.set_progress(0.0, 1.0);

			THEN("the require-view-update callback does not get called") {
				REQUIRE_FALSE(got_called);
			}
		}

		WHEN("its status is changed") {
			d.set_status(DlStatus::DOWNLOADING);

			THEN("the require-view-update callback gets called") {
				REQUIRE(got_called);
			}
		}

		WHEN("when its status is not changed") {
			d.set_status(DlStatus::QUEUED);

			THEN("the require-view-update callback does not get called") {
				REQUIRE_FALSE(got_called);
			}
		}
	}
}

TEST_CASE("filename() returns download's target filename", "[Download]")
{
	auto emptyCallback = []() {};

	Download d(emptyCallback);

	SECTION("filename returns empty string by default") {
		REQUIRE(d.filename().empty());
	}


	SECTION("filename returns same string which is set via set_filename") {
		d.set_filename("abc");
		REQUIRE(d.filename() == "abc");
	}

	SECTION("filename will return the latest configured filename") {
		d.set_filename("abc");
		d.set_filename("def");

		REQUIRE(d.filename() == "def");
	}
}

TEST_CASE("percents_finished() takes current progress into account",
	"[Download]")
{
	auto emptyCallback = []() {};

	Download d(emptyCallback);

	SECTION("percents_finished() returns 0 by default") {
		REQUIRE(d.percents_finished() == 0);
	}

	SECTION("percents_finished() updates according to set_progress's arguments") {
		double downloaded = 3.0;
		double total = 7.0;
		d.set_progress(downloaded, total);

		REQUIRE(d.percents_finished() == Approx(100.0 * (downloaded / total)));
	}

	SECTION("percents_finished() takes offset into account") {
		double offset = 5.0;
		double downloaded = 3.0;
		double total = 12.0;

		d.set_offset(offset);
		d.set_progress(downloaded, total);

		auto expected = Approx(100.0 * ((downloaded + offset) / (total + offset)));
		REQUIRE(d.percents_finished() == expected);
	}

	SECTION("percents_finished() returns 0 if total is unknown (0)") {
		double downloaded = 3.0;
		double total = 0.0;
		d.set_progress(downloaded, total);

		REQUIRE(d.percents_finished() == 0);
	}
}

TEST_CASE("basename() returns all text after last slash in the filename",
	"[Download]")
{
	auto emptyCallback = []() {};

	Download d(emptyCallback);

	SECTION("basename() returns empty string by default") {
		REQUIRE(d.basename().empty());
	}

	SECTION("basename() returns full filename if it does not contain slashes") {
		std::string filename = "lorem_ipsum.txt";
		d.set_filename(filename);

		REQUIRE(d.basename() == filename);
	}

	SECTION("basename() returns only text after the last slash in the filename") {
		std::string basename = "lorem_ipsum.txt";
		std::string path = "/test/path/";
		std::string filename = path + basename;
		d.set_filename(filename);

		REQUIRE(d.basename() == basename);
	}
}

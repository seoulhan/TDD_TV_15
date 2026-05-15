rem lcov --capture --directory build --base-directory . --output-file lcov.info
rem lcov --remove lcov.info "*/external/*" "*/googletest-1.16.0/*" --output-file lcov.info

call lcov --capture --directory build --base-directory . --gcov-tool gcov --output-file lcov.info
call lcov --extract lcov.info "*/src/*" "*/test/*" --output-file lcov.info
copy lcov.info .\build
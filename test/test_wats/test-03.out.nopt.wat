(module
  (import "env" "decent_wasm_test_log" (func $log (param i32)))
  (import "env" "decent_wasm_counter_exceed" (func $ctr_exceed (param i64)))
  (func $main_func
    (local $i i32)
    local.get 0
    i32.const 1
    i32.add
    local.set 0
    local.get 0
    call 0
    i64.const 11
    call 3
    block $blk_1
      loop $loop_1
        block $blk_2
          local.get 0
          i32.const 1
          i32.le_u
          i64.const 1
          call 3
          br_if 1 (;@2;)
          local.get 0
          i32.const 1
          i32.add
          local.set 0
          local.get 0
          i32.const 1
          i32.eq
          i64.const 2
          call 3
          br_if 0 (;@3;)
          local.get 0
          i32.const 1
          i32.add
          local.set 0
          local.get 0
          i32.const 1
          i32.eq
          i64.const 2
          call 3
          br_if 2 (;@1;)
        end
      end
    end
    block $blk_1
      loop $loop_1
        block $blk_2
          local.get 0
          i32.const 1
          i32.le_u
          i64.const 1
          call 3
          br_if 1 (;@2;)
          local.get 0
          i32.const 1
          i32.add
          local.set 0
          local.get 0
          i32.const 1
          i32.eq
          i64.const 2
          call 3
          br_if 0 (;@3;)
          local.get 0
          i32.const 1
          i32.add
          local.set 0
          local.get 0
          i32.const 1
          i32.eq
          i64.const 2
          call 3
          br_if 2 (;@1;)
        end
        local.get 0
        i32.const 1
        i32.add
        local.set 0
        i64.const 1
        call 3
      end
      local.get 0
      call 0
      i64.const 10
      call 3
    end
    local.get 0
    i32.const 1
    i32.add
    local.set 0
    local.get 0
    call 0
    i64.const 11
    call 3)
  (start 2)
  (type (;0;) (func (param i32)))
  (type (;1;) (func (param i32 i32) (result i32)))
  (type (;2;) (func))
  (global (;0;) (mut i64) (i64.const 0))
  (global (;1;) (mut i64) (i64.const 0))
  (type (;3;) (func (param i64)))
  (func (;3;) (param i64)
    local.get 0
    global.get 1
    i64.add
    global.set 1
    block  ;; label = @1
      global.get 1
      global.get 0
      i64.le_u
      br_if 0 (;@1;)
      global.get 1
      call 1
    end))

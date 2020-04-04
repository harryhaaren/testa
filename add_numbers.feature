Feature:
  Scenario Outline:
    Given a number <one>;
    And another number  <two>;
    When added together;
    Then the result must be <answer>:

    Examples:
      |    one    | two     | answer   |
      |     50    |   20    |    70    |
      | 0xefff    |  0x1    |  0xf000  |
      | 0xff0f    |  0x1    |  0xff10  |
